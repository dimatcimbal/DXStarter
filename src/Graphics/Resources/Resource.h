#pragma once

#include "Includes/ComIncl.h"
#include "Includes/GraphicsIncl.h"

class Resource {
   public:
    Resource(Microsoft::WRL::ComPtr<ID3D12Resource2>&& pResource)
        : mD3DResource{std::move(pResource)} {}

    // Prohibit copying
    Resource(const Resource& other) = delete;
    Resource& operator=(const Resource& other) = delete;

    Resource(Resource&& other) noexcept : mD3DResource{std::move(other.mD3DResource)} {}

    Resource& operator=(Resource&& other) noexcept {
        if (this != &other) {
            mD3DResource = std::move(other.mD3DResource);
        }
        return *this;
    }

    ID3D12Resource2* GetResource() const {
        return mD3DResource.Get();
    }

   protected:
    Microsoft::WRL::ComPtr<ID3D12Resource2> mD3DResource;
};
