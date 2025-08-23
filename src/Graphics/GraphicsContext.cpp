#include "GraphicsContext.h"

#include "DebugLayer.h"

bool GraphicsContext::Create(std::unique_ptr<GraphicsContext>& OutContext) {

    std::unique_ptr<DebugLayer> OutDebugLayer;
    if (!DebugLayer::Create(OutDebugLayer)) {
        LOG_ERROR(L"Failed to initialize the Debug Layer.\n");
        return false;
    }

    // TODO: create Device, CommandQueue, CommandAllocator, etc.

    OutContext = std::make_unique<GraphicsContext>(std::move(OutDebugLayer));
    return true;
}

bool GraphicsContext::Draw() {
    return true;
}
