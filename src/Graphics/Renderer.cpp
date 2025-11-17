#include "Renderer.h"

#include "CommandList10.h"
#include "Material/Material.h"
#include "RootSignature.h"

// Internal visitor implementation - not part of public API
class RenderObjectBuilder : public NodeVisitor {
   public:
    RenderObjectBuilder(CommandList10& Cmdl,
                        std::set<RenderingKey>& RenderingOrder,
                        std::vector<RenderingObject>& RenderingObjects)
        : Cmdl(Cmdl), mRenderingOrder(RenderingOrder), mRenderingObjects(RenderingObjects) {}

    void Visit(Node* node) override;

   private:
    CommandList10& Cmdl;
    std::set<RenderingKey>& mRenderingOrder;
    std::vector<RenderingObject>& mRenderingObjects;
};

void RenderObjectBuilder::Visit(Node* node) {
    // Skip all materialIds prior to kMaterialFirstId
    if (node->GetMaterialId() < kMaterialFirstId) {
        return;
    }

    // Skip the node if no mesh
    if (!node->GetMeshInstance()) {
        return;
    }

    // 2. Update Mesh constant buffers
    node->GetMeshInstance()->Update(Cmdl, node->GetWorldTransform());

    // 3. Build a RenderingKey
    RenderingKey rKey;
    // Sets mObjectId to the next index by mRenderingObjects.size(); mMaterialId and
    // mPass are set below.
    rKey.value = mRenderingObjects.size();  // NOTE: The size of rendering OBJECTS vector.
    // Setting the most significant bits
    rKey.mMaterialId = node->GetMaterialId();
    rKey.mPass = DrawPass::kOpaque;
    mRenderingOrder.insert(rKey);

    // 4. Build RenderingObject
    RenderingObject rObject(node->GetMeshInstance());
    mRenderingObjects.push_back(std::move(rObject));
}

// Internal visitor implementation - not part of public API
class WorldTransformVisitor : public NodeVisitor {
   public:
    void Visit(Node* node) override;
};

void WorldTransformVisitor::Visit(Node* node) {
    // Get accumulated parents' world transformation
    // and multiply it by current node's local one
    auto WorldTransform = node->GetParent() ? node->GetParent()->GetWorldTransform() : Matrix4();
    node->SetWorldTransform(WorldTransform * node->GetTransform());
}

// Renderer class
WorldTransformVisitor Renderer::sWorldTransformVisitor{};

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

        RenderObjectBuilder renderObjectBuilder(Cmdl, mRenderingOrder, mRenderingObjects);
        Node::TraverseDepthFirst(mScene,
                                 // Compute world transformation for each Node
                                 sWorldTransformVisitor,
                                 // Create rendering objects from the Node
                                 renderObjectBuilder);
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
        std::shared_ptr<Material> currentMaterial;

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
                if (!Material::GetMaterial(currentMaterialId, currentMaterial)) {
                    LOG_ERROR(L"Failed to draw a frame as material with materialId=%u is missing",
                              currentMaterialId);
                    return false;
                }

                Cmdl->SetPipelineState(currentMaterial->GetD3DPipelineState());
            }

            // Issue Draw commands
            mRenderingObjects[key.mObjectId].Draw(Cmdl);
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
