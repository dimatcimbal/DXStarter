#include "Device.h"

#include "Logging/Logging.h"

bool Device::Create(D3D_FEATURE_LEVEL FeatureLevel, bool IsHardwareDevice, bool HasMaxVideoMemory,
                    std::unique_ptr<Device>& OutDevice) {
    Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory;
    if FAILED (CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory))) {
        LOG_ERROR(L"Failed to create DXGI factory.\n");
        return false;
    }

    Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;
    Microsoft::WRL::ComPtr<ID3D12Device14> bestD3DDevice;

    SIZE_T maxVideoMemory{0};
    for (uint32_t i{0}; dxgiFactory->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND; ++i) {
        // Get adapter description
        DXGI_ADAPTER_DESC1 desc;
        adapter->GetDesc1(&desc);

        // Check is a hardware adapter
        if (IsHardwareDevice && (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)) {
            // we don't need the ComPtr anymore
            adapter.Reset();
            continue;
        }

        // Check adapter can create a D3D12 device
        Microsoft::WRL::ComPtr<ID3D12Device14> d3d12Device;
        if (FAILED(D3D12CreateDevice(adapter.Get(), FeatureLevel, IID_PPV_ARGS(&d3d12Device)))) {
            // we don't need the ComPtr anymore
            adapter.Reset();
            continue;
        }

        // Check adapter has the *most* dedicated video memory
        if (HasMaxVideoMemory) {
            if (desc.DedicatedVideoMemory > maxVideoMemory) {
                maxVideoMemory = desc.DedicatedVideoMemory;
                bestD3DDevice = std::move(d3d12Device);
            }
        } else {
            bestD3DDevice = std::move(d3d12Device);
        }
    }

    if (!bestD3DDevice) {
        LOG_ERROR(L"No suitable D3D12 device found.\n");
        return false;
    }

    OutDevice = std::make_unique<Device>(std::move(dxgiFactory), std::move(bestD3DDevice));
    return true;
}