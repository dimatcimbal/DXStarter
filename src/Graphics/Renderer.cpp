#include "Renderer.h"

#include "CommandList10.h"
#include "Device.h"
#include "Includes/GraphicsIncl.h"
#include "Logging/Logging.h"

bool Renderer::Create(Device* Device, std::unique_ptr<Renderer>& OutRenderer) {
    OutRenderer = std::make_unique<Renderer>(Device);
    return true;
}

bool Renderer::Tick(float deltaTime) const {
    if (mModel) {
        CommandList10 Cmdl;
        if (!mDevice->GetCommandList(Cmdl)) {
            LOG_ERROR("Failed to get command list for model update.\n");
            return false;
        }

        mModel->Tick(Cmdl, deltaTime);
    }

    return true;
}

bool Renderer::Draw() const {
    if (mSwapChain == nullptr) {
        LOG_ERROR(L"\tSwapChain is not initialized.\n");
        return false;
    }

    // Get a command list
    FrameCommandList10 Cmdl;
    if (!mDevice->GetFrameCommandList(*mSwapChain, Cmdl)) {
        LOG_ERROR(L"Failed to draw a frame.\n");
        return false;
    }

    if (mModel) {
        mModel->Draw(Cmdl);
    }

    // The command list gets closed and executed automatically on exiting the scope
    return true;
}

bool Renderer::Update() {
    // Calculate delta time
    LARGE_INTEGER CurrentTime;
    QueryPerformanceCounter(&CurrentTime);

    float DeltaTime = 0.0f;
    if (!mFirstFrame) {
        // Get frequency on first frame
        if (mFrequency.QuadPart == 0) {
            QueryPerformanceFrequency(&mFrequency);
        }

        // Calculate delta time in seconds
        DeltaTime = static_cast<float>(CurrentTime.QuadPart - mLastFrameTime.QuadPart) /
                    static_cast<float>(mFrequency.QuadPart);
    } else {
        // First frame - initialize timing
        QueryPerformanceFrequency(&mFrequency);
        mFirstFrame = false;
    }

    mLastFrameTime = CurrentTime;

    // Scene update
    if (!Tick(DeltaTime)) {
        LOG_ERROR("Failed to update the scene.\n");
        return false;
    }

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

void Renderer::OnWindowCreate(HWND hWnd) {
    LOG_INFO(L"Renderer::OnCreate with window handle %p\n", hWnd);
    mGraphicsHwnd = hWnd;
    mIsCreated = true;
}

void Renderer::OnWindowResize(int NewWidth, int NewHeight) {
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
