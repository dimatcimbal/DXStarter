#include "GraphicsContext.h"

#include "DebugLayer.h"

bool GraphicsContext::Create(std::unique_ptr<GraphicsContext>& OutContext) {
    std::unique_ptr<DebugLayer> outDebugLayer;
    if (!DebugLayer::Create(outDebugLayer)) {
        LOG_ERROR(L"Failed to initialize the Debug Layer.\n");
        return false;
    }

    // Find the best available D3D12 device
    std::unique_ptr<Device> outDevice;
    if (!Device::Create(GRAPHICS_FEATURE_LEVEL, true, true, outDevice)) {
        LOG_ERROR(L"Failed to create the Device.\n");
        return false;
    }

    // TODO: create Device, CommandQueue, CommandAllocator, etc.

    OutContext = std::make_unique<GraphicsContext>(std::move(outDebugLayer));
    return true;
}

bool GraphicsContext::Draw() {
    return true;
}
