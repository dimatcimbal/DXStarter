#pragma once

#include <memory>

#include "Includes/GraphicsIncl.h"
#include "PipelineState.h"

class MaterialRegistry;

using MaterialId = uint32_t;

// The materialId from which materials get registered
constexpr MaterialId kMaterialFirstId = 100;

/**
 * Material class that owns the rendering pipeline state (PSO).
 * Multiple mesh instances can share the same material.
 */
class Material {
   public:
    static bool Create(std::unique_ptr<PipelineState>&& PipelineState,
                       std::shared_ptr<Material>& OutMaterial);

    static bool GetMaterial(MaterialId MaterialId, std::shared_ptr<Material>& OutMaterial);

    Material(std::unique_ptr<PipelineState>&& PipelineState)
        : mPipelineState(std::move(PipelineState)) {}

    ~Material() = default;

    // Prohibit copying
    Material(const Material&) = delete;
    Material& operator=(const Material&) = delete;

    // Prohibit moving as the consistency of the MaterialRegistry could be corrupted with a move
    // operation
    Material(Material&& Other) = delete;
    Material& operator=(Material&& Other) = delete;

    ID3D12PipelineState* GetD3DPipelineState() const {
        return mPipelineState ? mPipelineState->GetD3DPipelineState() : nullptr;
    }

    MaterialId GetMaterialId() const {
        return mMaterialId;
    }

   private:
    friend class MaterialRegistry;

    std::unique_ptr<PipelineState> mPipelineState;
    MaterialId mMaterialId{0};
};
