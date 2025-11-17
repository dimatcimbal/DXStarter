#pragma once

#include <memory>
#include <unordered_map>

#include "Logging/Logging.h"
#include "Material.h"

class MaterialRegistry {
   public:
    static void RegisterMaterial(const std::shared_ptr<Material>& Material) {
        auto& registry = GetMaterialRegistry();

        Material->mMaterialId = sNextMaterialId;
        registry[sNextMaterialId] = Material;
        ++sNextMaterialId;
    }

    static bool GetMaterial(MaterialId MaterialId, std::shared_ptr<Material>& OutMaterial) {
        auto& registry = GetMaterialRegistry();
        auto material = registry.find(MaterialId);
        if (material == registry.end()) {
            LOG_ERROR(L"Failed to find material with materialId %u", MaterialId);
            return false;
        }

        OutMaterial = material->second;
        return true;
    }

   private:
    // Using the function-local static pattern (Meyer's Singleton). This ensures the registry is
    // initialized on first access and is thread-safe in C++11+.
    static std::unordered_map<MaterialId, std::shared_ptr<Material>>& GetMaterialRegistry() {
        static std::unordered_map<MaterialId, std::shared_ptr<Material>> registry;
        return registry;
    }

    static uint32_t sNextMaterialId;
};