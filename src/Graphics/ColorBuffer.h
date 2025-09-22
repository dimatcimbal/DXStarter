#pragma once

#include "Includes/ComIncl.h"
#include "Includes/GraphicsIncl.h"

class ColorBuffer {
   public:
    ColorBuffer(D3D12_CPU_DESCRIPTOR_HANDLE RTVHandle,
                Microsoft::WRL::ComPtr<ID3D12Resource2>&& pResource)
        : mRTVHandle{RTVHandle}, mResource(std::move(pResource)) {}

    ColorBuffer(const ColorBuffer&) = delete;
    ColorBuffer& operator=(const ColorBuffer&) = delete;

    ID3D12Resource2* GetD3DResource() const {
        return mResource.Get();
    }

    D3D12_CPU_DESCRIPTOR_HANDLE GetRTV() const {
        return mRTVHandle;
    }

   private:
    D3D12_CPU_DESCRIPTOR_HANDLE mRTVHandle;
    Microsoft::WRL::ComPtr<ID3D12Resource2> mResource;
};