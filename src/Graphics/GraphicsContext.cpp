#include "GraphicsContext.h"

#include "DebugLayer.h"
#include "Includes/ComIncl.h"
#include "Includes/GraphicsIncl.h"
#include "Logging/Logging.h"

bool GraphicsContext::Create(std::unique_ptr<GraphicsContext>& OutContext) {
    std::unique_ptr<DebugLayer> pDebugLayer;
    if (!DebugLayer::Create(pDebugLayer)) {
        LOG_ERROR(L"\tFailed to initialize the Debug Layer.\n");
        return false;
    }

    // Find the best available D3D12 device
    std::unique_ptr<Device> pDevice;
    if (!Device::Create(GRAPHICS_FEATURE_LEVEL, true, true, pDevice)) {
        LOG_ERROR(L"\tFailed to create the Device.\n");
        return false;
    }

    D3D12_COMMAND_LIST_TYPE CommandListType = D3D12_COMMAND_LIST_TYPE_DIRECT;

    std::unique_ptr<CommandQueue> pCommandQueue;
    if (!pDevice->CreateCommandQueue(CommandListType, D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
                                     D3D12_COMMAND_QUEUE_FLAG_NONE, D3D12_FENCE_FLAG_NONE,
                                     pCommandQueue)) {
        LOG_ERROR(L"\tFailed to create the Command Queue.\n");
        return false;
    }

    std::unique_ptr<CommandAllocator> pCommandAllocator;
    if (!pDevice->CreateCommandAllocator(CommandListType, D3D12_COMMAND_LIST_FLAG_NONE,
                                         pCommandAllocator)) {
        LOG_ERROR(L"\tFailed to create the Command Allocator.\n");
        return false;
    }

    std::unique_ptr<DescriptorHeap> pRTVHeap;
    if (!pDevice->CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, RTV_DESCRIPTOR_COUNT,
                                       pRTVHeap)) {
        LOG_ERROR(L"\tFailed to create the RTV Descriptor Heap.\n");
        return false;
    }

    // TODO: Remove this DescriptorHeap verification test
    D3D12_CPU_DESCRIPTOR_HANDLE testHandle;
    pRTVHeap->AllocateHandles(1, testHandle);
    LOG_INFO(L"\tAllocated RTV Descriptor Handle: %llu\n", testHandle.ptr);

    OutContext = std::make_unique<GraphicsContext>(
        std::move(pCommandQueue), std::move(pCommandAllocator), std::move(pRTVHeap),
        std::move(pDevice), std::move(pDebugLayer));
    return true;
}

bool GraphicsContext::GetCommandList(
    CommandList<ID3D12GraphicsCommandList10>& OutCommandList) const {
    ID3D12GraphicsCommandList10* pD3DCommandList;
    if (!mCommandAllocator->GetID3D12CommandList(pD3DCommandList)) {
        LOG_ERROR(L"Failed to get command list from the allocator.\n");
        return false;
    }

    OutCommandList = CommandList(mCommandQueue.get(), pD3DCommandList);
    return true;
}

bool GraphicsContext::CreateSwapChain(HWND hWnd, uint32_t Width, uint32_t Height) {
    LOG_INFO(L"\tCreating SwapChain for window %p with size %u x %u\n", hWnd, Width, Height);

    // Release the previous swap chain
    if (mSwapChain) {
        mSwapChain->FlushAll();
        mSwapChain.reset();
    }

    // DXGI_FORMAT_R8G8B8A8_UNORM is the most common swap chain format
    DXGI_FORMAT Format = DXGI_FORMAT_R8G8B8A8_UNORM;

    DXGI_USAGE Usage =
        // Back buffer (target for rendering)
        DXGI_USAGE_BACK_BUFFER |
        // output to a window (hwnd)
        DXGI_USAGE_RENDER_TARGET_OUTPUT;

    uint32_t Flags =
        // Create a flip-model swap chain
        DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH |
        // Skip vsync
        DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

    if (!mDevice->CreateSwapChain(hWnd, *mCommandQueue, Width, Height, SWAP_CHAIN_BUFFER_COUNT,
                                  Usage, Format, Flags, mSwapChain)) {
        LOG_ERROR(L"\tFailed to create a SwapChain");
        return false;
    }

    mDevice->DisableAltEnterFullscreenToggle(hWnd);
    return true;
}

bool GraphicsContext::ResizeSwapChain(uint32_t Width, uint32_t Height) {
    LOG_INFO(L"\tResizing SwapChain to %u x %u\n", Width, Height);

    if (mSwapChain == nullptr) {
        LOG_ERROR(
            L"\tSwapChain is not initialized. "
            "Use GraphicsContext::CreateSwapChain before presenting.\n");
        return false;
    }

    if (mWidth == Width && mHeight == Height) {
        // No change if the dimensions are the same which can happen when window is restored.
        LOG_INFO(L"\tNo change in swap chain dimensions. Window is restored.\n");
        return true;
    }

    if (!mSwapChain->Resize(Width, Height)) {
        LOG_ERROR(L"\tFailed to resize the swap chain.\n");
        return false;
    }

    // Update stored width and height
    mWidth = Width;
    mHeight = Height;

    return true;
}

bool GraphicsContext::Present() const {
    if (mSwapChain == nullptr) {
        LOG_ERROR(
            L"\tSwapChain is not initialized. "
            "Use GraphicsContext::CreateSwapChain before presenting.\n");
        return false;
    }

    if (!mSwapChain->Present()) {
        LOG_ERROR(L"\tFailed to present the swap chain.\n");
        return false;
    }
    return true;
}

bool GraphicsContext::FlushAll() const {
    if (mSwapChain == nullptr) {
        LOG_ERROR(
            L"\tSwapChain is not initialized. "
            "Use GraphicsContext::CreateSwapChain before presenting.\n");
        return false;
    }

    if (!mSwapChain->FlushAll()) {
        LOG_ERROR(L"\tFailed to flush the swap chain's graphics queue.\n");
        return false;
    }
    return true;
}
