#include "Renderer.h"

#include "CommandList10.h"
#include "Device.h"
#include "Includes/ComIncl.h"
#include "Includes/GraphicsIncl.h"
#include "Logging/Logging.h"
#include "SwapChain.h"

bool Renderer::LoadVertexData(const void* data, size_t size) const {
    // 1. Upload bytes to the upload buffer
    if (!mUploadBuffer->UploadBytes(data, size)) {
        LOG_ERROR(L"Failed to upload bytes to the upload buffer.\n");
        return false;
    }

    // 2. Get a command list
    CommandList10 cmdl;
    if (!mDevice->GetCommandList(cmdl)) {
        LOG_ERROR(L"Failed to get command list.\n");
        return false;
    }

    // 3. Transition the upload buffer to GENERIC_READ
    cmdl.TransitionResource(*mUploadBuffer,
                            // From state
                            D3D12_RESOURCE_STATE_COMMON,
                            // To state
                            D3D12_RESOURCE_STATE_GENERIC_READ);

    // 4. Transition the vertex buffer to COPY_DEST
    cmdl.TransitionResource(*mVertexBuffer,
                            // From state
                            D3D12_RESOURCE_STATE_COMMON,
                            // To state
                            D3D12_RESOURCE_STATE_COPY_DEST);

    // 5. Copy from the upload buffer to the vertex buffer
    cmdl.CopyBufferRegion(
        // From
        *mUploadBuffer, 0,
        // To
        *mVertexBuffer, size);

    return true;
}

bool Renderer::Create(size_t VertexBufferSize,
                      Device* pDevice,
                      std::unique_ptr<Renderer>& OutRenderer) {
    std::unique_ptr<DefaultBuffer> VertexBuffer;
    if (!pDevice->CreateBuffer(L"VertexBuffer", D3D12_HEAP_TYPE_DEFAULT, VertexBufferSize,
                               VertexBuffer)) {
        LOG_ERROR(L"Failed to create default buffer.\n");
        return false;
    }

    std::unique_ptr<UploadBuffer> UploadBuffer;
    if (!pDevice->CreateBuffer(L"UploadBuffer", D3D12_HEAP_TYPE_UPLOAD, VertexBufferSize,
                               UploadBuffer)) {
        LOG_ERROR(L"Failed to create upload buffer.\n");
        return false;
    }

    OutRenderer =
        std::make_unique<Renderer>(pDevice, std::move(VertexBuffer), std::move(UploadBuffer));
    return true;
}

bool Renderer::Draw() const {
    if (mSwapChain == nullptr) {
        LOG_ERROR(L"\tSwapChain is not initialized.\n");
        return false;
    }

    // Get a command list
    FrameCommandList10 cmdl;
    if (!mDevice->GetFrameCommandList(*mSwapChain, cmdl)) {
        LOG_ERROR(L"Failed to draw a frame.\n");
        return false;
    }

    cmdl->SetName(L"FrameCommandList");
    // TODO: draw something with cmd->DoSomething()

    // The command list gets closed and executed automatically on exiting the scope
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
