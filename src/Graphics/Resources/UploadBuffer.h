#pragma once

#include <utility>

#include "DefaultBuffer.h"
#include "Resource.h"

class BufferRange;

class UploadBuffer : public DefaultBuffer {
    friend BufferRange;

   public:
    UploadBuffer(D3D12_HEAP_TYPE Type,
                 size_t _265byteAlignedBufferSize,
                 Microsoft::WRL::ComPtr<ID3D12Resource2> pResource)
        : DefaultBuffer(Type, _265byteAlignedBufferSize, std::move(pResource)) {}

    // Prohibit copying
    UploadBuffer(UploadBuffer& other) = delete;
    UploadBuffer& operator=(UploadBuffer& other) = delete;

    // Instance members
    /**
     * Maps a range of the buffer and returns a BufferRange object that will unmap it when it goes
     * out of scope.
     */
    BufferRange Map(size_t Offset, size_t Size);

    bool UploadBytes(const void* data, size_t size);
};

/**
 * RAII class for mapping and unmapping an UploadBuffer.
 */
class BufferRange {
   public:
    BufferRange() = default;

    BufferRange(size_t Offset, size_t Size, UploadBuffer* Buffer)
        : mBuffer(Buffer), mD3DRange(CD3DX12_RANGE(Offset, Size)) {
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
        mPtr = std::exchange(other.mPtr, nullptr);
        mBuffer = std::exchange(other.mBuffer, nullptr);
    }

    BufferRange& operator=(BufferRange&& other) noexcept {
        if (this != &other) {
            mPtr = std::exchange(other.mPtr, nullptr);
            mBuffer = std::exchange(other.mBuffer, nullptr);
        }
        return *this;
    }

    void* GetPtr() const {
        return mPtr;
    }

   private:
    void* mPtr = nullptr;
    D3D12_RANGE mD3DRange;
    UploadBuffer* mBuffer = nullptr;
};
