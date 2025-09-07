#include "Device.h"

#include "CommandAllocator.h"
#include "CommandQueue.h"
#include "Logging/Logging.h"

bool Device::CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE Type,
                                    D3D12_COMMAND_LIST_FLAGS Flags,
                                    std::unique_ptr<CommandAllocator>& OutAllocator) const {
    using Microsoft::WRL::ComPtr;

    ComPtr<ID3D12CommandAllocator> pCommandAllocator;
    if (FAILED(mD3DDevice->CreateCommandAllocator(
            // Command list type (D3D12_COMMAND_LIST_TYPE_DIRECT /
            // D3D12_COMMAND_LIST_TYPE_COMPUTE, etc)
            Type,
            // Return value
            IID_PPV_ARGS(&pCommandAllocator)))) {
        LOG_ERROR(L"\t\tFailed to create D3D12 command allocator.\n");
        return false;
    }

    ComPtr<ID3D12GraphicsCommandList10> pGraphicsCommandList;
    if (FAILED(mD3DDevice->CreateCommandList1(
            // Current GPU
            0,
            // Command list type (D3D12_COMMAND_LIST_TYPE_DIRECT /
            // D3D12_COMMAND_LIST_TYPE_COMPUTE, etc)
            Type,
            // Command list flags (most likely D3D12_COMMAND_LIST_FLAG_NONE)
            Flags,
            // Return value
            IID_PPV_ARGS(&pGraphicsCommandList)))) {
        LOG_ERROR(L"\t\tFailed to create D3D12 command list.\n");
        return false;
    }

    OutAllocator = std::make_unique<CommandAllocator>(Type, std::move(pCommandAllocator),
                                                      std::move(pGraphicsCommandList));
    return true;
}

bool Device::Create(D3D_FEATURE_LEVEL FeatureLevel,
                    bool IsHardwareDevice,
                    bool HasMaxVideoMemory,
                    std::unique_ptr<Device>& OutDevice) {
    using Microsoft::WRL::ComPtr;

    ComPtr<IDXGIFactory7> pDXGIFactory;
    if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&pDXGIFactory)))) {
        LOG_ERROR(L"\t\tFailed to create DXGI factory.\n");
        return false;
    }

    ComPtr<IDXGIAdapter1> pAdapter;
    ComPtr<ID3D12Device14> pBestD3DDevice;

    SIZE_T maxVideoMemory{0};
    for (uint32_t i{0}; pDXGIFactory->EnumAdapters1(i, &pAdapter) != DXGI_ERROR_NOT_FOUND; ++i) {
        // Get adapter description
        DXGI_ADAPTER_DESC1 desc;
        pAdapter->GetDesc1(&desc);

        // Check is a hardware adapter
        if (IsHardwareDevice && (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)) {
            // we don't need the ComPtr anymore
            pAdapter.Reset();
            continue;
        }

        // Check adapter can create a D3D12 device
        Microsoft::WRL::ComPtr<ID3D12Device14> pD3D12Device;
        if (FAILED(D3D12CreateDevice(pAdapter.Get(), FeatureLevel, IID_PPV_ARGS(&pD3D12Device)))) {
            // we don't need the ComPtr anymore
            pAdapter.Reset();
            continue;
        }

        // Check adapter has the *most* dedicated video memory
        if (HasMaxVideoMemory) {
            if (desc.DedicatedVideoMemory > maxVideoMemory) {
                maxVideoMemory = desc.DedicatedVideoMemory;
                pBestD3DDevice = std::move(pD3D12Device);
            }
        } else {
            pBestD3DDevice = std::move(pD3D12Device);
        }
    }

    if (!pBestD3DDevice) {
        LOG_ERROR(L"\t\tNo suitable D3D12 device found.\n");
        return false;
    }

    OutDevice = std::make_unique<Device>(std::move(pDXGIFactory), std::move(pBestD3DDevice));
    return true;
}

bool Device::CreateCommandQueue(D3D12_COMMAND_LIST_TYPE Type,
                                D3D12_COMMAND_QUEUE_PRIORITY Priority,
                                D3D12_COMMAND_QUEUE_FLAGS QueueFlags,
                                D3D12_FENCE_FLAGS FenceFlags,
                                std::unique_ptr<CommandQueue>& OutQueue) const {
    using Microsoft::WRL::ComPtr;

    D3D12_COMMAND_QUEUE_DESC desc;
    desc.NodeMask = 0;  // pick the default gpu
    desc.Type = Type;
    desc.Priority = Priority;
    desc.Flags = QueueFlags;

    ComPtr<ID3D12CommandQueue> pD3D12CommandQueue;
    if (FAILED(mD3DDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&pD3D12CommandQueue)))) {
        LOG_ERROR(L"\t\tFailed to create D3D12 command queue.\n");
        return false;
    }

    // initial fence value
    uint64_t InitFenceValue = 0;

    // fence object
    ComPtr<ID3D12Fence1> pD3D12Fence;
    if (FAILED(mD3DDevice->CreateFence(InitFenceValue, FenceFlags, IID_PPV_ARGS(&pD3D12Fence)))) {
        LOG_ERROR(L"\t\tFailed to create ID3D12Fence1.\n");
        return false;
    }

    // Create an event handle for queue synchronization
    HANDLE EventHandle = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (EventHandle == nullptr) {
        LOG_ERROR(L"Failed to create event handle.\n");
        return false;
    }

    OutQueue = std::make_unique<CommandQueue>(
        Type, InitFenceValue, EventHandle, std::move(pD3D12Fence), std::move(pD3D12CommandQueue));

    return true;
}