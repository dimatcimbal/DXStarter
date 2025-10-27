#pragma once

#include "Resource.h"

class ByteBuffer : public Resource {
   public:
    ByteBuffer(D3D12_HEAP_TYPE Type,
               size_t _265byteAlignedBufferSize,
               Microsoft::WRL::ComPtr<ID3D12Resource2> pResource)
        : Resource(std::move(pResource)), mType(Type), mSize(_265byteAlignedBufferSize) {}

    // Prohibit copying
    ByteBuffer(const ByteBuffer& other) = delete;
    ByteBuffer& operator=(const ByteBuffer& other) = delete;

    // Move constructor
    ByteBuffer(ByteBuffer&& other) noexcept
        : Resource(std::move(other)),
          mType(std::exchange(other.mType, D3D12_HEAP_TYPE_DEFAULT)),
          mSize(std::exchange(other.mSize, 0)) {}

    // Move assignment operator
    ByteBuffer& operator=(ByteBuffer&& other) noexcept {
        if (this != &other) {
            // Handle only the derived class's own members
            mType = std::exchange(other.mType, D3D12_HEAP_TYPE_DEFAULT);
            mSize = std::exchange(other.mSize, 0);

            // Call parent's move assignment operator to handle inherited members
            Resource::operator=(std::move(other));
        }
        return *this;
    }

    // Getters/Setters
    size_t GetBufferSize() const {
        return mSize;
    }

   protected:
    D3D12_HEAP_TYPE mType;
    size_t mSize;
};
