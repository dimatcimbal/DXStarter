#pragma once

#include "Includes/ComIncl.h"
#include "Includes/GraphicsIncl.h"

class Resource {
    friend class CommandList10;

   public:
    Resource(D3D12_RESOURCE_STATES State, Microsoft::WRL::ComPtr<ID3D12Resource2>&& pD3DResource)
        : mState(State),
          mDeviceVirtualAddress(pD3DResource->GetGPUVirtualAddress()),
          mD3DResource{std::move(pD3DResource)} {}

    virtual ~Resource() = default;

    // Prohibit copying
    Resource(const Resource& other) = delete;
    Resource& operator=(const Resource& other) = delete;

    // Allow moving
    Resource(Resource&& other) noexcept
        : mState(std::exchange(other.mState, D3D12_RESOURCE_STATES_NULL)),
          mDeviceVirtualAddress(
              std::exchange(other.mDeviceVirtualAddress, D3D12_GPU_VIRTUAL_ADDRESS_NULL)),
          mD3DResource{std::exchange(other.mD3DResource, nullptr)} {}

    Resource& operator=(Resource&& other) noexcept {
        if (this != &other) {
            mState = std::exchange(other.mState, D3D12_RESOURCE_STATES_NULL);
            mDeviceVirtualAddress =
                std::exchange(other.mDeviceVirtualAddress, D3D12_GPU_VIRTUAL_ADDRESS_NULL);
            mD3DResource = std::exchange(other.mD3DResource, nullptr);
        }
        return *this;
    }

    D3D12_GPU_VIRTUAL_ADDRESS GetDeviceVirtualAddress() const {
        return mDeviceVirtualAddress;
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
    // The resource usage state
    D3D12_RESOURCE_STATES mState;
    D3D12_GPU_VIRTUAL_ADDRESS mDeviceVirtualAddress;
    Microsoft::WRL::ComPtr<ID3D12Resource2> mD3DResource;
};
