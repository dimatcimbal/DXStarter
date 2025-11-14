#pragma once

#include "Includes/ComIncl.h"
#include "Includes/GraphicsIncl.h"

class Resource {
   public:
    Resource(Microsoft::WRL::ComPtr<ID3D12Resource2>&& pD3DResource)
        : mState(D3D12_RESOURCE_STATES_NULL),
          mGpuVirtualAddress(pD3DResource->GetGPUVirtualAddress()),
          mD3DResource{std::move(pD3DResource)} {}

    virtual ~Resource() = default;

    // Prohibit copying
    Resource(const Resource& other) = delete;
    Resource& operator=(const Resource& other) = delete;

    // Allow moving
    Resource(Resource&& other) noexcept
        : mState(std::exchange(other.mState, D3D12_RESOURCE_STATES_NULL)),
          mGpuVirtualAddress(
              std::exchange(other.mGpuVirtualAddress, D3D12_GPU_VIRTUAL_ADDRESS_NULL)),
          mD3DResource{std::exchange(other.mD3DResource, nullptr)} {}

    Resource& operator=(Resource&& other) noexcept {
        if (this != &other) {
            mState = std::exchange(other.mState, D3D12_RESOURCE_STATES_NULL);
            mGpuVirtualAddress =
                std::exchange(other.mGpuVirtualAddress, D3D12_GPU_VIRTUAL_ADDRESS_NULL);
            mD3DResource = std::exchange(other.mD3DResource, nullptr);
        }
        return *this;
    }

    D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const {
        return mGpuVirtualAddress;
    }

    ID3D12Resource2* GetResource() const {
        return mD3DResource.Get();
    }

    D3D12_RESOURCE_STATES GetCurrentState() const {
        return mState;
    }

    void SetCurrentState(D3D12_RESOURCE_STATES NewState) {
        mState = NewState;
    }

   protected:
    D3D12_RESOURCE_STATES mState;
    D3D12_GPU_VIRTUAL_ADDRESS mGpuVirtualAddress;
    Microsoft::WRL::ComPtr<ID3D12Resource2> mD3DResource;
};
