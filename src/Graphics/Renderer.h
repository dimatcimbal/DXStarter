#pragma once

#include <memory>
#include <utility>

#include "Logging/Logging.h"
#include "Mesh/Mesh.h"
#include "Mesh/MeshInstance.h"
#include "PipelineState.h"
#include "RootSignature.h"
#include "SwapChain.h"

// Forward declarations
class Device;

/**
 * High-level renderer class. Manages Rendering Pipeline state.
 */
class Renderer {
   public:
    static bool Create(Device& Device, std::unique_ptr<Renderer>& OutRenderer);

    Renderer(std::unique_ptr<RootSignature>&& RootSignature,
             std::unique_ptr<PipelineState>&& PipelineState)
        : mRootSignature(std::move(RootSignature)), mPSO(std::move(PipelineState)) {}

    ~Renderer() {
        LOG_INFO(L"\tFreeing Renderer.\n");
    }

    // Prohibit copying
    Renderer(const Renderer& copy) = delete;
    Renderer& operator=(const Renderer& copy) = delete;

    // Allow moving
    Renderer(Renderer&& other) noexcept
        : mPSO(std::exchange(other.mPSO, nullptr)),
          mRootSignature(std::exchange(other.mRootSignature, nullptr)),
          mModel(std::exchange(other.mModel, nullptr)),
          mViewport(other.mViewport),
          mScissorRect(other.mScissorRect) {}

    Renderer& operator=(Renderer&& other) noexcept {
        if (this != &other) {
            mPSO = std::exchange(other.mPSO, nullptr);
            mRootSignature = std::exchange(other.mRootSignature, nullptr);
            mModel = std::exchange(other.mModel, nullptr);
            mViewport = other.mViewport;
            mScissorRect = other.mScissorRect;
        }
        return *this;
    }

    // Instance members

    /**
     * Draws a frame.
     * @return
     */
    bool Draw(CommandList10& Cmdl) const;

    /**
     * Main loop tick function.
     * @param deltaTime Time elapsed since last tick in seconds.
     * @return True if the renderer should continue running, false to exit.
     */
    bool Update(CommandList10& Cmdl, float deltaTime) const;

    void Resize(uint32_t Width, uint32_t Height);

    // Getters/Setters

    void SetModel(std::unique_ptr<MeshInstance>&& Model) {
        mModel = std::move(Model);
    }

   private:
    std::unique_ptr<PipelineState> mPSO;
    std::unique_ptr<RootSignature> mRootSignature;
    std::unique_ptr<MeshInstance> mModel;

    D3D12_VIEWPORT mViewport;
    RECT mScissorRect;
};