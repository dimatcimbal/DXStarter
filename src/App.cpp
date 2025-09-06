
#include "App.h"

#include "Graphics/GraphicsContext.h"
#include "Logging/Logging.h"

bool App::Create(App*& OutApp) {
    // Create the MainWindow instance before creating the window
    std::unique_ptr<GraphicsContext> pGraphicsContext;
    if (!GraphicsContext::Create(pGraphicsContext)) {
        LOG_ERROR(L"Failed to create GraphicsContext.\n");
        return false;
    }

    OutApp = new App(std::move(pGraphicsContext));
    return true;
}

bool App::Update() {
    // Scene update and the rendering part

    // TODO: Scene update

    if (mIsMinimized) {
        mIsMinimized = false;
        // Skip rendering when the window is minimized
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

        if (!mGraphicsContext->ResizeSwapChain(mWigth, mHeight)) {
            LOG_ERROR(L"Failed while calling GraphicsContext::ResizeSwapChain with %d x %d.\n",
                      mWigth, mHeight);
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