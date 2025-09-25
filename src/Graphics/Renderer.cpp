#include "Renderer.h"

#include "CommandList10.h"
#include "Includes/ComIncl.h"
#include "Includes/GraphicsIncl.h"
#include "Logging/Logging.h"

bool Renderer::Draw() const {
    if (mSwapChain == nullptr) {
        LOG_ERROR(L"\tSwapChain is not initialized.\n");
        return false;
    }

    // Get a command list
    CommandList10 cmdlist;
    if (!mDevice->GetCommandList(*mSwapChain, cmdlist)) {
        LOG_ERROR(L"Failed to draw a frame.\n");
        return false;
    }

    // TODO: cmd->DoSomething() calls
    // cmdlist->SetName(L"Graphics Command List");

    // The command list gets closed and executed automatically
    return true;
}

void Renderer::OnCreate(HWND hWnd) {
    LOG_INFO(L"Renderer::OnCreate with window handle %p\n", hWnd);
    mGraphicsHwnd = hWnd;
    mIsCreated = true;
}

void Renderer::OnResize(int NewWidth, int NewHeight) {
    LOG_INFO(L"Renderer::OnResize to %d x %d\n", NewWidth, NewHeight);

    if (NewWidth == 0 || NewHeight == 0) {
        // Window is minimized or has zero area; ignore resize
        mIsMinimized = true;
        return;
    }

    mNewWidth = static_cast<uint32_t>(NewWidth);
    mNewHeight = static_cast<uint32_t>(NewHeight);

    mIsMinimized = false;
    mIsResized = true;
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

        if (!mDevice->CreateSwapChain(mGraphicsHwnd, mWidth, mHeight, mSwapChain)) {
            LOG_ERROR(L"Failed while calling Device::CreateSwapChain handler.\n");
            return false;
        }
    }

    // Resizing window; there might be lots of resize messages, so we only resize once per frame
    if (mIsResized) {
        mIsResized = false;

        if (mWidth == mNewWidth && mHeight == mNewHeight) {
            LOG_INFO(L"Skip resizing to the same size %d x %d.\n", mWidth, mHeight);
            return true;
        }

        // Caching new window size
        mWidth = mNewWidth;
        mHeight = mNewHeight;

        LOG_INFO(L"Resizing to %d x %d.\n", mWidth, mHeight);
        if (!mSwapChain->Resize(mWidth, mHeight)) {
            LOG_ERROR(L"Failed while calling SwapChain::Resize with %d x %d.\n", mWidth, mHeight);
            return false;
        }
    }

    // Do draw
    if (!Draw()) {
        LOG_ERROR(L"Failed to draw a frame.\n");
        return false;
    }

    // Present the frame with the swap chain
    if (!mSwapChain->Present()) {
        LOG_ERROR(L"Failed to present a frame.\n");
        return false;
    }

    return mIsRunning;
}
