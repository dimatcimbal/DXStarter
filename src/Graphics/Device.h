#pragma once
#include <wrl/client.h>

#include <memory>

#include "Includes/GraphicsIncl.h"
#include "Logging/Logging.h"

class Device {
   public:
    /**
     *
     * @param FeatureLevel
     * @param IsHardwareDevice
     * @param HasMaxVideoMemory
     * @param OutDevice
     * @return
     */
    static bool Create(D3D_FEATURE_LEVEL FeatureLevel, bool IsHardwareDevice,
                       bool HasMaxVideoMemory, std::unique_ptr<Device>& OutDevice);

    Device(Microsoft::WRL::ComPtr<IDXGIFactory7>&& DXGIFactory,
           Microsoft::WRL::ComPtr<ID3D12Device14> D3DDevice)
        : mDXGIFactory(std::move(DXGIFactory)), mD3DDevice(std::move(D3DDevice)) {};

    ~Device() {
        LOG_INFO(L"Freeing Device.\n");
    };

    // Deleted copy constructor and assignment operator to prevent copying
    Device(Device& copy) = delete;
    Device& operator=(const Device& copy) = delete;

   private:
    Microsoft::WRL::ComPtr<IDXGIFactory7> mDXGIFactory;
    Microsoft::WRL::ComPtr<ID3D12Device14> mD3DDevice;
};
