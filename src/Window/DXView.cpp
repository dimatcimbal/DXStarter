#include "DXView.h"

#include "Graphics/CommandList10.h"
#include "Graphics/Device.h"
#include "Graphics/Renderer.h"
#include "Logging/Logging.h"

void DXView::OnWindowCreate(HWND hWnd) {
    LOG_INFO(L"Renderer::OnCreate with window handle %p\n", hWnd);
    mGraphicsHwnd = hWnd;
    mIsCreating = true;
    mIsRunning = true;
}

void DXView::OnWindowResize(int NewWidth, int NewHeight) {
    LOG_INFO(L"Renderer::OnResize to %d x %d\n", NewWidth, NewHeight);

    if (NewWidth == 0 || NewHeight == 0) {
        // Window is minimized or has zero area; ignore resize
        mIsMinimizing = true;
        return;
    }

    mNewWidth = static_cast<uint32_t>(NewWidth);
    mNewHeight = static_cast<uint32_t>(NewHeight);

    mIsMinimizing = false;
    mIsResizing = true;
}

bool DXView::Update() {
    // Calculate delta time
    LARGE_INTEGER CurrentTime;
    QueryPerformanceCounter(&CurrentTime);

    float DeltaTime = 0.0f;
    if (mLastFrameTime.QuadPart != 0) {
        // Calculate delta time in seconds
        // mLastFrameTime.QuadPart == 0 indicates first frame (no previous time available)
        DeltaTime = static_cast<float>(CurrentTime.QuadPart - mLastFrameTime.QuadPart) /
                    static_cast<float>(mFrequency.QuadPart);
    }

    mLastFrameTime = CurrentTime;

    {  // Scene update
        CommandList10 Cmdl;
        if (!mDevice.GetCommandList(Cmdl)) {
            LOG_ERROR(L"Failed to get command list for model update.\n");
            return false;
        }

        if (!mRenderer.Update(Cmdl, DeltaTime)) {
            LOG_ERROR(L"Failed to update the scene.\n");
            return false;
        }
    }

    // Skip rendering when the window is minimized
    if (mIsMinimizing) {
        // The mIsMinimizing flag gets reset in OnResize method
        return true;
    }

    // Creating graphics resources
    if (mIsCreating) {
        mIsCreating = false;
        // a graphics initialization logic could be placed here
    }

    // Resizing window; there might be lots of resize messages, so we only resize once per frame
    if (mIsResizing) {
        mIsResizing = false;

        if (mWidth == mNewWidth && mHeight == mNewHeight) {
            LOG_INFO(L"Skip resizing to the same size %d x %d.\n", mWidth, mHeight);
            return true;
        }

        // Caching new window size
        mWidth = mNewWidth;
        mHeight = mNewHeight;

        // Postponed creation of the graphic resources
        if (!mSwapChain) {
            LOG_INFO("Creating the SwapChain of the size %d x %d.\n", mWidth, mHeight);
            if (!mDevice.CreateSwapChain(mGraphicsHwnd, mWidth, mHeight, mSwapChain)) {
                LOG_ERROR(L"Failed while calling Device::CreateSwapChain handler.\n");
                return false;
            }
        }

        LOG_INFO(L"Resizing to %d x %d.\n", mWidth, mHeight);
        if (!mSwapChain->Resize(mWidth, mHeight)) {
            LOG_ERROR(L"Failed while calling SwapChain::Resize with %d x %d.\n", mWidth, mHeight);
            return false;
        }

        // Update viewport and scissor rect to match new swap chain size
        mRenderer.Resize(mWidth, mHeight);
    }

    {  // Scene draw
        if (mSwapChain == nullptr) {
            LOG_ERROR(L"\tSwapChain is not initialized.\n");
            return false;
        }

        // Get a command list
        FrameCommandList10 Cmdl;
        if (!mDevice.GetFrameCommandList(*mSwapChain, Cmdl)) {
            LOG_ERROR(L"Failed to draw a frame.\n");
            return false;
        }

        // Do draw
        if (!mRenderer.Draw(Cmdl)) {
            LOG_ERROR(L"Failed to draw a frame.\n");
            return false;
        }
    }

    // Present the frame with the swap chain
    if (!mSwapChain->Present()) {
        LOG_ERROR(L"Failed to present a frame.\n");
        return false;
    }

    return mIsRunning;
}