#pragma once

#include <cstddef>
#include <filesystem>
#include <memory>

class ByteBuffer {
   public:
    static bool Create(std::filesystem::path FilePath, std::unique_ptr<ByteBuffer>& OutBuffer);

    /** Aligns the given size to 256 bytes, which is required for constant buffers in D3D12.
     * @param Size The size to align.
     * @return The aligned size, rounded up to the nearest multiple of 256 bytes.
     */
    static size_t AlignTo256Bytes(size_t Size) {
        return (Size + 255) & ~255;
    }

    ByteBuffer(size_t Size, std::unique_ptr<std::byte[]>&& BufferData)
        : mBuffer(std::move(BufferData)), mSize(Size) {}

    ~ByteBuffer() = default;

    // Prohibit copying
    ByteBuffer(const ByteBuffer& Other) = delete;
    ByteBuffer& operator=(const ByteBuffer& Other) = delete;

    // Allow moving
    ByteBuffer(ByteBuffer&& Other) noexcept
        : mBuffer(std::move(Other.mBuffer)), mSize(std::exchange(Other.mSize, 0)) {}

    ByteBuffer& operator=(ByteBuffer&& Other) noexcept {
        if (this != &Other) {
            mBuffer = std::move(Other.mBuffer);
            mSize = std::exchange(Other.mSize, 0);
        }
        return *this;
    }

    // Getters/Setters
    const void* GetBuffer() const {
        return mBuffer.get();
    }

    size_t GetSize() const {
        return mSize;
    }

   private:
    std::unique_ptr<std::byte[]> mBuffer;
    size_t mSize;
};
