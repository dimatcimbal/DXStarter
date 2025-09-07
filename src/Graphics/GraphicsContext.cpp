#include "GraphicsContext.h"

#include "DebugLayer.h"

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

    OutContext =
        std::make_unique<GraphicsContext>(std::move(pCommandQueue), std::move(pCommandAllocator),
                                          std::move(pDevice), std::move(pDebugLayer));
    return true;
}

bool GraphicsContext::CreateSwapChain(HWND hWnd, uint32_t Width, uint32_t Height) {
    LOG_INFO(L"GraphicsContext::CreateSwapChain is being called for HWND: 0x%p\n", hWnd);
    return true;
}

bool GraphicsContext::ResizeSwapChain(uint32_t Width, uint32_t Height) {
    LOG_INFO(L"GraphicsContext::ResizeSwapChain is being called for Width: %d, Height: %d\n", Width,
             Height);
    return true;
}

bool GraphicsContext::Draw() const {
    ID3D12GraphicsCommandList10* CommandList;
    if (!mCommandAllocator->GetID3D12CommandList(CommandList)) {
        LOG_ERROR(L"Failed to get command list from the allocator.\n");
        return false;
    }

    CommandList cmd(mCommandQueue.get(), pD3DCommandList);
    // TODO: Record commands into CommandList here

    if (!mCommandQueue->ExecuteCommandList(CommandList)) {
        LOG_ERROR(L"\tFailed to execute command list.\n");
        return false;
    }

    if (!mCommandQueue->WaitForIdle()) {
        LOG_ERROR(L"\tFailed to wait on command queue.\n");
        return false;
    }

    return true;
}

bool GraphicsContext::FlushAll() const {
    return mCommandQueue->WaitForIdle();
}
