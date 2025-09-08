#include "SwapChain.h"

#include "Logging/Logging.h"

bool SwapChain::Present() const {
    uint32_t SyncInterval = 1;  // On the next vertical blank
    uint32_t Flags = 0;         // No special flags
    if (FAILED(mDXGISwapChain->Present(SyncInterval, Flags))) {
        LOG_ERROR(L"\t\tFailed to present the swap chain.\n");
        return false;
    }
    return true;
}

bool SwapChain::Resize(uint32_t Width, uint32_t Height) const {
    LOG_INFO(L"\t\tResizing swap chain buffers to %u x %u.\n", Width, Height);
    if (FAILED(mDXGISwapChain->ResizeBuffers(mBufferCount, Width, Height, mFormat, mFlags))) {
        LOG_ERROR(L"\t\tFailed to resize swap chain buffers.\n");
        return false;
    }
    return true;
}

bool SwapChain::FlushAll() const {
    LOG_INFO(L"\t\tFlushing the bound command queues.\n");
    if (!mGraphicsQueue.WaitForIdle()) {
        LOG_ERROR(L"\t\tFailed to flush the command queue.\n");
        return false;
    }
    return true;
}