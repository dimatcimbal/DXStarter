#pragma once

#include "Resource.h"

class DeviceBuffer : public Resource {
   public:
    DeviceBuffer(D3D12_HEAP_TYPE Type,
                 D3D12_RESOURCE_STATES State,
                 size_t _265byteAlignedBufferSize,
                 Microsoft::WRL::ComPtr<ID3D12Resource2> pResource)
        : Resource(State, std::move(pResource)), mType(Type), mSize(_265byteAlignedBufferSize) {}

    virtual ~DeviceBuffer() = default;

    // Prohibit copying
    DeviceBuffer(const DeviceBuffer& other) = delete;
    DeviceBuffer& operator=(const DeviceBuffer& other) = delete;

    // Move constructor
    DeviceBuffer(DeviceBuffer&& other) noexcept
        : Resource(std::move(other)),
          mType(std::exchange(other.mType, D3D12_HEAP_TYPE_DEFAULT)),
          mSize(std::exchange(other.mSize, 0)) {}

    // Move assignment operator
    DeviceBuffer& operator=(DeviceBuffer&& other) noexcept {
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
