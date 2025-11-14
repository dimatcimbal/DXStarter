#include "Renderer.h"

#include "CommandList10.h"

bool Renderer::Create(std::unique_ptr<Renderer>& OutRenderer) {
    OutRenderer = std::make_unique<Renderer>();
    OutRenderer->SetClearColorRGBA(0.4f, 0.6f, 0.9f, 1.0f);
    return true;
}

bool Renderer::Update(CommandList10& Cmdl, float deltaTime) const {
    if (mModel) {
        // Issue Update command
        mModel->Update(Cmdl, deltaTime);
    }

    return true;
}

bool Renderer::Draw(FrameCommandList10& Cmdl) const {
    if (mModel) {
        // The FIRST thing is to CLEAR the render target
        Cmdl.ClearRenderTarget(mClearColorRGBA);

        // Set viewport and scissor rect
        Cmdl->RSSetViewports(1, &mViewport);
        Cmdl->RSSetScissorRects(1, &mScissorRect);

        // Issue Draw commands (Material sets PSO and RootSignature)
        mModel->Draw(Cmdl);
    }

    // The command list gets closed and executed automatically on exiting the scope
    return true;
}

void Renderer::Resize(uint32_t Width, uint32_t Height) {
    // Viewport: Maps normalized device coordinates (-1 to 1) to pixel coordinates
    // Typically matches the full render target size
    mViewport.TopLeftX = 0.0f;
    mViewport.TopLeftY = 0.0f;
    mViewport.Width = static_cast<float>(Width);
    mViewport.Height = static_cast<float>(Height);
    mViewport.MinDepth = 0.0f;  // Near plane (closest to camera)
    mViewport.MaxDepth = 1.0f;  // Far plane (farthest from camera)

    // Scissor Rect: Clips pixels outside this rectangle (in pixel coordinates)
    // Typically matches the viewport, but can be smaller for UI overlays, etc.
    mScissorRect.left = 0;
    mScissorRect.top = 0;
    mScissorRect.right = static_cast<LONG>(Width);
    mScissorRect.bottom = static_cast<LONG>(Height);
}
