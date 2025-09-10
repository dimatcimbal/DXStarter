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

bool SwapChain::BuffersReadTo(std::vector<ComPtr<ID3D12Resource2>>& OutVector) const {
    LOG_INFO(L"\t\tReading swap chain buffers.\n");
    for (uint32_t i = 0; i < mSwapChainBufferCount; ++i) {
        if (FAILED(mDXGISwapChain->GetBuffer(i, IID_PPV_ARGS(&OutVector[i])))) {
            LOG_ERROR(L"\t\tFailed to get IDXGISwapChain buffer %d.\n", i);
            return false;
        }
    }
    return true;
}

void SwapChain::BuffersRelease() {
    LOG_INFO(L"\t\tReleasing swap chain buffers.\n");
    for (uint32_t i = 0; i < mSwapChainBufferCount; ++i) {
        if (mSwapChainBuffers[i]) {
            mSwapChainBuffers[i].Reset();
        }
    }
}

bool SwapChain::Resize(uint32_t Width, uint32_t Height) {
    // Flush all GPU work and release references to the buffers.
    FlushAll();
    
    // Resize the swap chain buffers.
    LOG_INFO(L"\t\tResizing swap chain buffers to %u x %u.\n", Width, Height);
    if (FAILED(mDXGISwapChain->ResizeBuffers(mSwapChainBufferCount, Width, Height, mFormat, mFlags))) {
        LOG_ERROR(L"\t\tFailed to resize swap chain buffers.\n");
        return false;
    }

    // Read the new swap chain buffers into mBuffers.
    if (!BuffersReadTo(mSwapChainBuffers)) {
        LOG_ERROR(L"\t\tFailed to read swap chain buffers after resizing.\n");
        return false;
    }

    return true;
}

bool SwapChain::FlushAll() {
    LOG_INFO(L"\t\tFlushing the bound command queues.\n");

    // Complete all GPU work.
    if (!mGraphicsQueue.WaitForIdle()) {
        LOG_ERROR(L"\t\tFailed to flush the command queue.\n");
        return false;
    }

    // Clean the references to the buffers.
    BuffersRelease();

    return true;
}