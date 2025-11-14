#pragma once

#include <memory>

#include "Includes/GraphicsIncl.h"
#include "PipelineState.h"
#include "RootSignature.h"

// Forward declarations
class Device;

/**
 * Material class that owns the rendering pipeline state (PSO) and root signature.
 * Multiple mesh instances can share the same material.
 */
class Material {
   public:
    Material(std::unique_ptr<RootSignature>&& RootSignature,
             std::unique_ptr<PipelineState>&& PipelineState)
        : mRootSignature(std::move(RootSignature)), mPSO(std::move(PipelineState)) {}

    ~Material() = default;

    // Prohibit copying
    Material(const Material&) = delete;
    Material& operator=(const Material&) = delete;

    // Allow moving
    Material(Material&& other) noexcept
        : mRootSignature(std::move(other.mRootSignature)), mPSO(std::move(other.mPSO)) {}

    Material& operator=(Material&& other) noexcept {
        if (this != &other) {
            mRootSignature = std::move(other.mRootSignature);
            mPSO = std::move(other.mPSO);
        }
        return *this;
    }

    ID3D12RootSignature* GetD3DRootSignature() const {
        return mRootSignature ? mRootSignature->GetD3DRootSignature() : nullptr;
    }

    ID3D12PipelineState* GetD3DPipelineState() const {
        return mPSO ? mPSO->GetD3DPipelineState() : nullptr;
    }

   private:
    std::unique_ptr<RootSignature> mRootSignature;
    std::unique_ptr<PipelineState> mPSO;
};
