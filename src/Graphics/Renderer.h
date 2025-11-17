#pragma once

#include <memory>
#include <set>
#include <utility>

#include "CommandList10.h"
#include "Graphics/Scene/Node.h"
#include "IO/ByteBuffer.h"
#include "Includes/GraphicsIncl.h"
#include "Logging/Logging.h"
#include "Math/Matrix.h"
#include "Mesh/MeshInstance.h"

// Forward declarations
class Device;

enum DrawPass {
    kOpaque,
    kNumPasses,
};

/**
 * Scene Node representation optimized for rendering
 */
struct RenderingObject {
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
          mSceneTransform(),
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
        : mRootSignature(std::exchange(Other.mRootSignature, nullptr)),
          mScene(std::exchange(Other.mScene, nullptr)),
          mScissorRect(Other.mScissorRect),
          mViewport(Other.mViewport) {
        std::ranges::copy(Other.mClearColorRGBA, mClearColorRGBA);
    }

    Renderer& operator=(Renderer&& Other) noexcept {
        if (this != &Other) {
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
     * @return
     */
    bool Draw(FrameCommandList10& Cmdl) const;

    /**
     * Main loop tick function.
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
    RootSignature* mRootSignature;

    // Rendering cache
    std::set<RenderingKey> mRenderingOrder{};
    std::vector<MeshInstance*> mRenderingObjects{};

    float mClearColorRGBA[4];
    Node* mScene;

    // Default Scene transform like zoom for instance
    Matrix4 mSceneTransform;
    D3D12_VIEWPORT mViewport;
    RECT mScissorRect;
};
