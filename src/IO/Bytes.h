#pragma once

#include <cstddef>
#include <filesystem>
#include <memory>

class Bytes {
   public:
    static bool Load(std::filesystem::path path, std::unique_ptr<Bytes>& OutBuffer);

    /** Aligns the given size to 256 bytes, which is required for constant buffers in D3D12.
     * @param Size The size to align.
     * @return The aligned size, rounded up to the nearest multiple of 256 bytes.
     */
    static size_t AlignTo256Bytes(size_t Size) {
        return (Size + 255) & ~255;
    }

    ~Bytes() = default;

    // Prohibit copying
    Bytes(const Bytes& other) = delete;
    Bytes& operator=(const Bytes& other) = delete;

    // Allow moving
    Bytes(Bytes&& other) noexcept
        : mBuffer(std::move(other.mBuffer)), mSize(std::exchange(other.mSize, 0)) {}
    Bytes& operator=(Bytes&& other) noexcept {
        if (this != &other) {
            mBuffer = std::move(other.mBuffer);
            mSize = std::exchange(other.mSize, 0);
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
    Bytes(size_t Size, std::unique_ptr<std::byte[]>&& Buffer)
        : mBuffer(std::move(Buffer)), mSize(Size) {}

    std::unique_ptr<std::byte[]> mBuffer;
    size_t mSize;
};
