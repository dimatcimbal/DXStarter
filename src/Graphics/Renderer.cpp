#include "Renderer.h"

#include "CommandList10.h"
#include "Device.h"
#include "Material/Material.h"
#include "Math/Matrix.h"

bool Renderer::Create(RootSignature& RootSignature, std::unique_ptr<Renderer>& OutRenderer) {
    OutRenderer = std::make_unique<Renderer>(RootSignature);
    OutRenderer->SetClearColorRGBA(0.4f, 0.6f, 0.9f, 1.0f);
    return true;
}

bool Renderer::Update(CommandList10& Cmdl, float DeltaTime) {
    if (mScene) {
        // Clear rendering caches
        mRenderingOrder.clear();
        mRenderingObjects.clear();

        Node::TraverseDepthFirst(mScene, [this, &Cmdl, DeltaTime](Node* Node) {
            // 1. Update node (and mesh constant buffers if there is a mesh)
            Node->Update(Cmdl, DeltaTime);

            // Skip materials prior to kMaterialFirstId
            if (Node->GetMaterialId() < kMaterialFirstId) {
                return;
            }

            // Skip the node if no mesh
            if (!Node->GetMeshInstance()) {
                return;
            }

            // 2. Rebuild both RenderingObjects and RenderingKeys
            RenderingKey key;
            // Sets mObjectId to the next index by mRenderingObjects.size(); mMaterialId and mPass
            // are set below.
            key.value = mRenderingObjects.size();  // NOTE: The size of rendering OBJECTS vector.
            // Setting the most significant bits
            key.mMaterialId = Node->GetMaterialId();
            key.mPass = DrawPass::kOpaque;
            mRenderingObjects.push_back(Node->GetMeshInstance());
            mRenderingOrder.insert(key);
        });
    }

    return true;
}

bool Renderer::Draw(FrameCommandList10& Cmdl) const {
    if (mScene) {
        // The FIRST thing is to CLEAR the render target
        Cmdl.ClearRenderTarget(mClearColorRGBA);

        Cmdl->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        Cmdl->SetGraphicsRootSignature(mRootSignature->GetD3DRootSignature());

        // Set viewport and scissor rect
        Cmdl->RSSetViewports(1, &mViewport);
        Cmdl->RSSetScissorRects(1, &mScissorRect);

        DrawPass currentPass{};
        MaterialId currentMaterialId{0};
        for (const RenderingKey& key : mRenderingOrder) {
            // DrawPass switch
            if (currentPass != key.mPass) {
                currentPass = static_cast<DrawPass>(key.mPass);

                // Update the context if needed
            }

            // Material switch
            if (currentMaterialId != key.mMaterialId) {
                currentMaterialId = key.mMaterialId;

                // Next material
                auto material = Material::GetMaterial(currentMaterialId);
                Cmdl->SetPipelineState(material->GetD3DPipelineState());
            }

            // Issue Draw commands
            if (auto* mesh = mRenderingObjects[key.mObjectId]) {
                mesh->Draw(Cmdl);
            }
        }
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
