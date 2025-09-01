
#include "WindowState.h"

#include "Graphics/GraphicsContext.h"
#include "Logging/Logging.h"

bool WindowState::Update() {
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

    // Draw a frame
    if (!mGraphicsContext->Draw()) {
        LOG_ERROR(L"Failed to draw a frame.\n");
        return false;
    }

    return mIsRunning;
}