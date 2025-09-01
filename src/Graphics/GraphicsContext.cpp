#include "GraphicsContext.h"

#include "DebugLayer.h"

bool GraphicsContext::Create(std::unique_ptr<GraphicsContext>& OutContext) {
    std::unique_ptr<DebugLayer> pDebugLayer;
    if (!DebugLayer::Create(pDebugLayer)) {
        LOG_ERROR(L"Failed to initialize the Debug Layer.\n");
        return false;
    }

    // Find the best available D3D12 device
    std::unique_ptr<Device> pDevice;
    if (!Device::Create(GRAPHICS_FEATURE_LEVEL, true, true, pDevice)) {
        LOG_ERROR(L"Failed to create the Device.\n");
        return false;
    }

    std::unique_ptr<CommandQueue> pCommandQueue;
    if (!pDevice->CreateCommandQueue(
            D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
            D3D12_COMMAND_QUEUE_FLAG_NONE, D3D12_FENCE_FLAG_NONE, pCommandQueue)) {
        LOG_ERROR(L"Failed to create the Command Queue.\n");
        return false;
    }

    // TODO: create Device, CommandQueue, CommandAllocator, etc.

    OutContext = std::make_unique<GraphicsContext>(std::move(pCommandQueue), std::move(pDevice),
                                                   std::move(pDebugLayer));
    return true;
}

bool GraphicsContext::OnWindowCreate(HWND hWnd, uint32_t Width, uint32_t Height) {
    LOG_INFO(L"GraphicsContext::OnWindowCreate is being called\n");
    return true;
}

bool GraphicsContext::OnWindowResize(uint32_t Width, uint32_t Height) {
    LOG_INFO(L"GraphicsContext::OnWindowResize is being called\n");
    return true;
}

bool GraphicsContext::Draw() {
    return true;
}
