#pragma once

#include <utility>

#include "Includes/ComIncl.h"
#include "Includes/GraphicsIncl.h"

class RootSignature {
   public:
    RootSignature(Microsoft::WRL::ComPtr<ID3D12RootSignature> RootSignature)
        : mD3DRootSignature(std::move(RootSignature)) {}

    // Prohibit copying
    RootSignature(const RootSignature& other) = delete;
    RootSignature& operator=(const RootSignature& other) = delete;

    // Allow moving
    RootSignature(RootSignature&& other) noexcept
        : mD3DRootSignature(std::exchange(other.mD3DRootSignature, nullptr)) {}

    RootSignature& operator=(RootSignature&& other) noexcept {
        if (this != &other) {
            mD3DRootSignature = std::exchange(other.mD3DRootSignature, nullptr);
        }
        return *this;
    }

    ID3D12RootSignature* GetD3DRootSignature() const {
        return mD3DRootSignature ? mD3DRootSignature.Get() : nullptr;
    }

   private:
    Microsoft::WRL::ComPtr<ID3D12RootSignature> mD3DRootSignature;
};
