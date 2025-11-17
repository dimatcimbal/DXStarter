#pragma once

#include <atomic>
#include <memory>
#include <unordered_map>

#include "Includes/GraphicsIncl.h"
#include "PipelineState.h"
#include "RootSignature.h"

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
                       std::shared_ptr<Material>& OutMaterial) {
        auto material = std::make_shared<Material>(std::move(PipelineState));
        material->mMaterialId = sNextMaterialId.fetch_add(1, std::memory_order_relaxed);
        GetMaterialRegistry()[material->mMaterialId] = material;

        OutMaterial = material;
        return true;
    }

    Material(std::unique_ptr<PipelineState>&& PipelineState)
        : mPipelineState(std::move(PipelineState)) {}

    static std::shared_ptr<Material> GetMaterial(MaterialId MaterialId) {
        return GetMaterialRegistry()[MaterialId];
    }

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
    // Using the function-local static pattern (Meyer's Singleton). This ensures the registry is
    // initialized on first access and is thread-safe in C++11+.
    static std::unordered_map<MaterialId, std::shared_ptr<Material>>& GetMaterialRegistry() {
        static std::unordered_map<MaterialId, std::shared_ptr<Material>> registry;
        return registry;
    }

    static std::atomic<MaterialId> sNextMaterialId;

    std::unique_ptr<PipelineState> mPipelineState;
    MaterialId mMaterialId{0};
};
