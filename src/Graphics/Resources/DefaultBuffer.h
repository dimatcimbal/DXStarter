#pragma once

#include "Resource.h"

class DefaultBuffer : public Resource {
   public:
    DefaultBuffer(D3D12_HEAP_TYPE Type,
                  size_t _265byteAlignedBufferSize,
                  Microsoft::WRL::ComPtr<ID3D12Resource2> pResource)
        : Resource(std::move(pResource)), mType(Type), mSize(_265byteAlignedBufferSize) {}

    // Prohibit copying
    DefaultBuffer(const DefaultBuffer& other) = delete;
    DefaultBuffer& operator=(const DefaultBuffer& other) = delete;

   protected:
    D3D12_HEAP_TYPE mType;
    size_t mSize;
};
