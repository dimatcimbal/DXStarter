#include "Material.h"

// Material Registry initialization
std::atomic<MaterialId> Material::sNextMaterialId{kMaterialFirstId};