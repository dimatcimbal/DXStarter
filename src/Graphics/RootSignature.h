#pragma once

#include "Includes/ComIncl.h"
#include "Includes/GraphicsIncl.h"

class RootSignature {
   public:
    RootSignature(Microsoft::WRL::ComPtr<ID3D12RootSignature> RootSignature)
        : mD3DRootSignature(std::move(RootSignature)) {}

    ID3D12RootSignature* GetD3DRootSignature() const {
        return mD3DRootSignature.Get();
    }

   private:
    Microsoft::WRL::ComPtr<ID3D12RootSignature> mD3DRootSignature;
};
