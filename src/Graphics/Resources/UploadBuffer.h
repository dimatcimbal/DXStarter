#pragma once

#include <utility>

#include "ByteBuffer.h"
#include "Resource.h"

class BufferRange;

/**
 * UploadBuffer CPU bound buffer class. Can map a region of the buffer for writing.
 */
class UploadBuffer : public ByteBuffer {
    friend BufferRange;

   public:
    UploadBuffer(D3D12_HEAP_TYPE Type,
                 size_t _265byteAlignedBufferSize,
                 Microsoft::WRL::ComPtr<ID3D12Resource2> pResource)
        : ByteBuffer(Type, _265byteAlignedBufferSize, std::move(pResource)) {}

    // Prohibit copying
    UploadBuffer(UploadBuffer& other) = delete;
    UploadBuffer& operator=(UploadBuffer& other) = delete;

    // Allow moving
    UploadBuffer(UploadBuffer&& other) noexcept : ByteBuffer(std::move(other)) {}

    UploadBuffer& operator=(UploadBuffer&& other) noexcept {
        if (this != &other) {
            ByteBuffer::operator=(std::move(other));
        }
        return *this;
    }

    /**
     * Maps a range of the buffer and returns a BufferRange object that will unmap it when it goes
     * out of scope.
     */
    BufferRange Map(size_t Offset, size_t Size);
    BufferRange Map();

    bool UploadBytes(size_t Size, const void* data);
};

/**
 * RAII class for mapping and unmapping an UploadBuffer.
 */
class BufferRange {
   public:
    BufferRange() = default;

    BufferRange(size_t Offset, size_t Size, UploadBuffer* Buffer)
        : mBuffer(Buffer), mSize(Size), mD3DRange(CD3DX12_RANGE(Offset, Size)) {
        Buffer->GetResource()->Map(0, &mD3DRange, &mPtr);
    }

    ~BufferRange() {
        if (mBuffer) {
            mBuffer->GetResource()->Unmap(0, &mD3DRange);
        }
    }

    // Prohibit copying
    BufferRange(const BufferRange&) = delete;
    BufferRange& operator=(const BufferRange&) = delete;

    // Allow moving
    BufferRange(BufferRange&& other) noexcept {
        mSize = std::exchange(other.mSize, 0);
        mD3DRange = std::exchange(other.mD3DRange, {});
        mPtr = std::exchange(other.mPtr, nullptr);
        mBuffer = std::exchange(other.mBuffer, nullptr);
    }

    BufferRange& operator=(BufferRange&& other) noexcept {
        if (this != &other) {
            mSize = std::exchange(other.mSize, 0);
            mD3DRange = std::exchange(other.mD3DRange, {});
            mPtr = std::exchange(other.mPtr, nullptr);
            mBuffer = std::exchange(other.mBuffer, nullptr);
        }
        return *this;
    }

    bool UploadBytes(size_t size, const void* data) const;

    void* GetPtr() const {
        return mPtr;
    }

   private:
    size_t mSize{0};
    D3D12_RANGE mD3DRange;
    void* mPtr = nullptr;
    UploadBuffer* mBuffer = nullptr;
};
