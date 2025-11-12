#include "SwapChain.h"

#include "CommandList10.h"
#include "CommandQueue.h"
#include "Device.h"
#include "Includes/ComIncl.h"
#include "Includes/GraphicsIncl.h"
#include "Logging/Logging.h"

bool SwapChain::BuffersReadTo(std::vector<std::unique_ptr<ColorBuffer>>& OutVector) const {
    LOG_INFO(L"\t\tReading swap chain buffers.\n");
    for (uint32_t i = 0; i < mBackBufferCount; ++i) {
        ComPtr<ID3D12Resource2> pResource;
        if (FAILED(mDXGISwapChain->GetBuffer(i, IID_PPV_ARGS(&pResource)))) {
            LOG_ERROR(L"\t\tFailed to get IDXGISwapChain buffer %d.\n", i);
            return false;
        }

        D3D12_RENDER_TARGET_VIEW_DESC desc{};
        desc.Format = mFormat;
        desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
        desc.Texture2D.MipSlice = 0;
        desc.Texture2D.PlaneSlice = 0;

        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle;
        mDevice.CreateRenderTargetView(pResource.Get(), desc, rtvHandle);

        OutVector[i] = std::make_unique<ColorBuffer>(rtvHandle, std::move(pResource));
    }
    return true;
}

bool SwapChain::Present() const {
    uint32_t SyncInterval = 1;  // On the next vertical blank (VSync enabled)
    uint32_t Flags = 0;         // No special flags
    if (FAILED(mDXGISwapChain->Present(SyncInterval, Flags))) {
        LOG_ERROR(L"\t\tFailed to present the swap chain.\n");
        return false;
    }
    return true;
}

bool SwapChain::Resize(uint32_t Width, uint32_t Height) {
    // Flush all GPU work and release references to the buffers.
    FlushAll();

    // Resize the swap chain buffers.
    LOG_INFO(L"\t\tResizing swap chain buffers to %u x %u.\n", Width, Height);
    if (FAILED(mDXGISwapChain->ResizeBuffers(mBackBufferCount, Width, Height, mFormat, mFlags))) {
        LOG_ERROR(L"\t\tFailed to resize swap chain buffers.\n");
        return false;
    }

    // Read the new swap chain buffers into mBuffers.
    if (!BuffersReadTo(mBackBuffers)) {
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

    LOG_INFO(L"\t\tReleasing swap chain buffers.\n");
    // Release references to the buffers but keep the vector size intact.
    for (uint32_t i = 0; i < mBackBufferCount; ++i) {
        mBackBuffers[i] = nullptr;
    }
    return true;
}

void SwapChain::BeginFrame(FrameCommandList10& Cmdl) {
    // Read the current back buffer index from mDXGISwapChain.
    mCurrentBackBufferIndex = mDXGISwapChain->GetCurrentBackBufferIndex();

    Cmdl.TransitionResource(GetCurrentBackBuffer(), D3D12_RESOURCE_STATE_PRESENT,
                            D3D12_RESOURCE_STATE_RENDER_TARGET);

    // TODO: Remove this
    float ClearColorRGBA[] = {0.4f, 0.6f, 0.9f, 1.0f};
    Cmdl.ClearTarget(GetCurrentBackBuffer().GetRTV(), ClearColorRGBA);

    Cmdl.SetRenderTarget(GetCurrentBackBuffer().GetRTV());
}

void SwapChain::EndFrame(FrameCommandList10& Cmd) {
    Cmd.TransitionResource(GetCurrentBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET,
                           D3D12_RESOURCE_STATE_PRESENT);
}
