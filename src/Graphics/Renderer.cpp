#include "Renderer.h"

#include "CommandList.h"
#include "GraphicsContext.h"
#include "Includes/ComIncl.h"
#include "Includes/GraphicsIncl.h"
#include "Logging/Logging.h"

bool Renderer::Draw() const {
    // Get a command list
    CommandList<ID3D12GraphicsCommandList10> cmdlist;
    if (!mGraphicsContext->GetCommandList(cmdlist)) {
        LOG_ERROR(L"Failed to draw a frame.\n");
        return false;
    }

    // TODO: cmd->DoSomething() calls
    cmdlist->SetName(L"Graphics Command List");

    // The command list gets closed and executed automatically
    return true;
}

bool Renderer::Update() {
    // Scene update and the rendering part
    // TODO: Scene update

    // Skip rendering when the window is minimized
    if (mIsMinimized) {
        // The flag gets reset in OnResize
        return true;
    }

    // Creating graphics resources
    if (mIsCreated) {
        mIsCreated = false;

        if (!mGraphicsContext->CreateSwapChain(mGraphicsHwnd)) {
            LOG_ERROR(L"Failed while calling GraphicsContext::CreateSwapChain handler.\n");
            return false;
        }
    }

    // Resizing window; there might be lots of resize messages, so we only resize once per frame
    if (mIsResized) {
        mIsResized = false;

        if (!mGraphicsContext->ResizeSwapChain(mWidth, mHeight)) {
            LOG_ERROR(L"Failed while calling GraphicsContext::ResizeSwapChain with %d x %d.\n",
                      mWidth, mHeight);
            return false;
        }
    }

    // Do draw
    if (!Draw()) {
        LOG_ERROR(L"Failed to draw a frame.\n");
        return false;
    }

    // Present the frame with the swap chain
    if (!mGraphicsContext->Present()) {
        LOG_ERROR(L"Failed to present a frame.\n");
        return false;
    }

    return mIsRunning;
}
