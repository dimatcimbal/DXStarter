#pragma once

#include "Includes/ComIncl.h"
#include "Includes/GraphicsIncl.h"
#include "Resource.h"

class ColorBuffer : public Resource {
   public:
    ColorBuffer(D3D12_CPU_DESCRIPTOR_HANDLE RTVHandle,
                Microsoft::WRL::ComPtr<ID3D12Resource2>&& pResource)
        : Resource(std::move(pResource)), mRTVHandle{RTVHandle} {}

    // Prohibit copying
    ColorBuffer(const ColorBuffer&) = delete;
    ColorBuffer& operator=(const ColorBuffer&) = delete;

    D3D12_CPU_DESCRIPTOR_HANDLE GetRTV() const {
        return mRTVHandle;
    }

   private:
    D3D12_CPU_DESCRIPTOR_HANDLE mRTVHandle;
};