#include "Device.h"

#include <vector>

#include "CommandAllocator.h"
#include "CommandQueue.h"
#include "DescriptorHeap.h"
#include "Logging/Logging.h"

using Microsoft::WRL::ComPtr;

bool Device::Create(D3D_FEATURE_LEVEL FeatureLevel,
                    bool IsHardwareDevice,
                    bool HasMaxVideoMemory,
                    std::unique_ptr<Device>& OutDevice) {
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
        ComPtr<ID3D12Device14> pD3D12Device;
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

    D3D12_DESCRIPTOR_HEAP_TYPE rtvDescType = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

    D3D12_DESCRIPTOR_HEAP_DESC rtvDesc;
    rtvDesc.Type = rtvDescType;
    rtvDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    rtvDesc.NodeMask = 0;
    rtvDesc.NumDescriptors = RTV_DESCRIPTOR_COUNT;

    ComPtr<ID3D12DescriptorHeap> pDescriptorHeap;
    if (FAILED(pBestD3DDevice->CreateDescriptorHeap(&rtvDesc, IID_PPV_ARGS(&pDescriptorHeap)))) {
        LOG_ERROR(L"\t\tFailed to create D3D12 descriptor heap.\n");
        return false;
    }

    uint32_t DescriptorSize = pBestD3DDevice->GetDescriptorHandleIncrementSize(rtvDescType);

    std::unique_ptr<DescriptorHeap> pRTVHeap = std::make_unique<DescriptorHeap>(
        rtvDescType, DescriptorSize, RTV_DESCRIPTOR_COUNT, std::move(pDescriptorHeap));

    OutDevice = std::make_unique<Device>(std::move(pDXGIFactory), std::move(pBestD3DDevice),
                                         std::move(pRTVHeap));
    return true;
}

bool Device::CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE Type,
                                    D3D12_COMMAND_LIST_FLAGS Flags,
                                    std::unique_ptr<CommandAllocator>& OutAllocator) const {
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

bool Device::CreateCommandQueue(D3D12_COMMAND_LIST_TYPE Type,
                                D3D12_COMMAND_QUEUE_PRIORITY Priority,
                                D3D12_COMMAND_QUEUE_FLAGS QueueFlags,
                                D3D12_FENCE_FLAGS FenceFlags,
                                std::unique_ptr<CommandQueue>& OutQueue) const {
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

bool Device::CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE Type,
                                  uint32_t Count,
                                  std::unique_ptr<DescriptorHeap>& OutHeap) const {
    D3D12_DESCRIPTOR_HEAP_DESC desc;
    desc.Type = Type;
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    desc.NodeMask = 0;
    desc.NumDescriptors = Count;

    ComPtr<ID3D12DescriptorHeap> pDescriptorHeap;
    if (FAILED(mD3DDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&pDescriptorHeap)))) {
        LOG_ERROR(L"\t\tFailed to create D3D12 descriptor heap.\n");
        return false;
    }

    uint32_t DescriptorSize = mD3DDevice->GetDescriptorHandleIncrementSize(Type);

    OutHeap =
        std::make_unique<DescriptorHeap>(Type, DescriptorSize, Count, std::move(pDescriptorHeap));
    return true;
}

bool Device::CreateSwapChain(HWND hWnd,
                             CommandQueue& GraphicsQueue,
                             uint32_t Width,
                             uint32_t Height,
                             uint32_t BufferCount,
                             DXGI_USAGE BufferUsage,
                             DXGI_FORMAT Format,
                             uint32_t Flags,
                             std::unique_ptr<SwapChain>& OutSwapChain) {
    // Swap chain desc
    DXGI_SWAP_CHAIN_DESC1 desc;
    desc.Width = Width;
    desc.Height = Height;
    desc.Format = Format;
    desc.Flags = Flags;

    // No stereo
    desc.Stereo = FALSE;

    // no multi-sampling, pixel per pixel
    desc.SampleDesc.Count = 1;
    // no anti-aliasing
    desc.SampleDesc.Quality = 0;

    desc.BufferCount = BufferCount;
    desc.BufferUsage = BufferUsage;
    // Stretch the image until the SwapChain::Resize is called
    desc.Scaling = DXGI_SCALING_STRETCH;

    // Flip discard
    desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

    // Always ignore
    desc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

    // Fullscreen desc
    DXGI_SWAP_CHAIN_FULLSCREEN_DESC descFullscreen;
    descFullscreen.Windowed = true;

    // Create the swap chain as IDXGISwapChain1 first
    ComPtr<IDXGISwapChain1> pSwapChain1;
    if (FAILED(mDXGIFactory->CreateSwapChainForHwnd(GraphicsQueue.GetD3D12CommandQueue(), hWnd,
                                                    &desc, &descFullscreen, nullptr,
                                                    &pSwapChain1))) {
        LOG_ERROR(L"\t\tFailed to create DXGI swap chain.\n");
        return false;
    }

    // Query for IDXGISwapChain4 as we need it for GetCurrentBackBufferIndex()
    ComPtr<IDXGISwapChain4> pDXGISwapChain;
    if (FAILED(pSwapChain1.As(&pDXGISwapChain))) {
        LOG_ERROR(L"\t\tFailed to query IDXGISwapChain4 interface.\n");
        return false;
    }

    if (BufferCount < 2) {
        LOG_ERROR(L"\t\tSwap chain buffer count must be at least 2.\n");
        return false;
    }

    OutSwapChain = std::make_unique<SwapChain>(BufferCount, Format, Flags, *this, GraphicsQueue,
                                               std::move(pDXGISwapChain));
    return true;
}

void Device::CreateRTV(ID3D12Resource2* pResource,
                       D3D12_RENDER_TARGET_VIEW_DESC& desc,
                       D3D12_CPU_DESCRIPTOR_HANDLE& OutHandle) const {
    // Allocate a descriptor from the heap
    mRTVHeap->AllocateHandles(1, OutHandle);
    // Create the RTV
    mD3DDevice->CreateRenderTargetView(pResource, &desc, OutHandle);
}