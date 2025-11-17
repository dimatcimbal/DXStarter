#pragma once

#include <memory>
#include <set>
#include <utility>

#include "CommandList10.h"
#include "Includes/GraphicsIncl.h"
#include "Logging/Logging.h"
#include "Mesh/MeshInstance.h"
#include "Scene/Node.h"

// Forward declarations
class Device;
class RootSignature;
class WorldTransformVisitor;

enum DrawPass {
    kOpaque,
};

/**
 * Scene Node representation optimized for rendering
 */
class RenderingObject {
   public:
    RenderingObject() = default;
    RenderingObject(MeshInstance* mesh) : mMeshInstance(mesh) {}
    ~RenderingObject() = default;

    RenderingObject(const RenderingObject&) = delete;
    RenderingObject& operator=(const RenderingObject&) = delete;

    RenderingObject(RenderingObject&& other) noexcept
        : mMeshInstance(std::exchange(other.mMeshInstance, nullptr)) {}

    RenderingObject& operator=(RenderingObject&& other) noexcept {
        if (this != &other) {
            mMeshInstance = std::exchange(other.mMeshInstance, nullptr);
        }
        return *this;
    }

    void Draw(FrameCommandList10& Cmdl) const {
        mMeshInstance->Draw(Cmdl);
    }

   private:
    // Not-owning pointer; Onership belongs to the scene Node
    MeshInstance* mMeshInstance;
};

struct RenderingKey {
    union {
        uint64_t value;
        struct {
            uint64_t mObjectId : 28;    // bits 0-27 (LSB - least significant bits)
            uint64_t mMaterialId : 32;  // bits 28-59
            uint64_t mPass : 4;         // bits 60-63 (MSB - most significant bits)
        };
    };

    bool operator<(const RenderingKey& Other) const {
        return value < Other.value;
    }
};

/**
 * High-level renderer class. Manages viewport, scissor rect, and clear color.
 * Coordinates rendering of mesh instances.
 */
class Renderer {
   public:
    static bool Create(RootSignature& RootSignature, std::unique_ptr<Renderer>& OutRenderer);

    Renderer(RootSignature& RootSignature)
        : mRootSignature(&RootSignature),
          mClearColorRGBA{0.f, 0.f, 0.f, 1.f},
          mScene(nullptr),
          mScissorRect(),
          mViewport() {}

    ~Renderer() {
        LOG_INFO(L"Freeing Renderer.\n");
    }

    // Prohibit copying
    Renderer(const Renderer& Copy) = delete;
    Renderer& operator=(const Renderer& Copy) = delete;

    // Allow moving
    Renderer(Renderer&& Other) noexcept
        : mRenderingObjects(std::exchange(Other.mRenderingObjects, {})),
          mRenderingOrder(std::exchange(Other.mRenderingOrder, {})),
          mRootSignature(std::exchange(Other.mRootSignature, nullptr)),
          mScene(std::exchange(Other.mScene, nullptr)),
          mScissorRect(Other.mScissorRect),
          mViewport(Other.mViewport) {
        std::ranges::copy(Other.mClearColorRGBA, mClearColorRGBA);
    }

    Renderer& operator=(Renderer&& Other) noexcept {
        if (this != &Other) {
            mRenderingObjects = std::exchange(Other.mRenderingObjects, {});
            mRenderingOrder = std::exchange(Other.mRenderingOrder, {});
            mRootSignature = std::exchange(Other.mRootSignature, nullptr);
            mScene = std::exchange(Other.mScene, nullptr);
            mScissorRect = Other.mScissorRect;
            mViewport = Other.mViewport;
            std::ranges::copy(Other.mClearColorRGBA, mClearColorRGBA);
        }
        return *this;
    }

    // Instance members

    /**
     * Draws a frame.
     * @param Cmdl Frame command list to record draw commands into.
     * @return true if the frame was successfully drawn, false otherwise.
     */
    bool Draw(FrameCommandList10& Cmdl) const;

    /**
     * Main loop tick function.
     * @param Cmdl Command list to record update commands into.
     * @param DeltaTime Time elapsed since last tick in seconds.
     * @return True if the renderer should continue running, false to exit.
     */
    bool Update(CommandList10& Cmdl, float DeltaTime);

    void Resize(uint32_t Width, uint32_t Height);

    // Getters/Setters
    void SetClearColorRGBA(float R, float G, float B, float A) {
        mClearColorRGBA[0] = R;
        mClearColorRGBA[1] = G;
        mClearColorRGBA[2] = B;
        mClearColorRGBA[3] = A;
    }

    void SetScene(Node& Scene) {
        mScene = &Scene;
    }

   private:
    static WorldTransformVisitor sWorldTransformVisitor;

    RootSignature* mRootSignature;

    // Rendering cache
    std::set<RenderingKey> mRenderingOrder{};
    std::vector<RenderingObject> mRenderingObjects{};

    float mClearColorRGBA[4];
    Node* mScene;

    D3D12_VIEWPORT mViewport;
    RECT mScissorRect;
};
