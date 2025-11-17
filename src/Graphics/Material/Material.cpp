#include "Material.h"

#include "MaterialRegistry.h"

bool Material::Create(std::unique_ptr<PipelineState>&& PipelineState,
                      std::shared_ptr<Material>& OutMaterial) {
    auto material = std::make_shared<Material>(std::move(PipelineState));
    MaterialRegistry::RegisterMaterial(material);

    OutMaterial = material;
    return true;
}

bool Material::GetMaterial(MaterialId MaterialId, std::shared_ptr<Material>& OutMaterial) {
    return MaterialRegistry::GetMaterial(MaterialId, OutMaterial);
}