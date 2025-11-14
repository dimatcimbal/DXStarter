#pragma once

#include <algorithm>
#include <memory>
#include <utility>

#include "CommandList10.h"
#include "Includes/GraphicsIncl.h"
#include "Logging/Logging.h"
#include "Mesh/MeshInstance.h"

// Forward declarations
class Device;

/**
 * High-level renderer class. Manages viewport, scissor rect, and clear color.
 * Coordinates rendering of mesh instances.
 */
class Renderer {
   public:
    static bool Create(std::unique_ptr<Renderer>& OutRenderer);

    Renderer() : mClearColorRGBA{0.f, 0.f, 0.f, 1.f}, mViewport{}, mScissorRect{} {}

    ~Renderer() {
        LOG_INFO(L"\tFreeing Renderer.\n");
    }

    // Prohibit copying
    Renderer(const Renderer& copy) = delete;
    Renderer& operator=(const Renderer& copy) = delete;

    // Allow moving
    Renderer(Renderer&& other) noexcept
        : mModel(std::exchange(other.mModel, nullptr)),
          mViewport(other.mViewport),
          mScissorRect(other.mScissorRect) {
        std::ranges::copy(other.mClearColorRGBA, mClearColorRGBA);
    }

    Renderer& operator=(Renderer&& other) noexcept {
        if (this != &other) {
            mModel = std::exchange(other.mModel, nullptr);
            mViewport = other.mViewport;
            mScissorRect = other.mScissorRect;
            std::ranges::copy(other.mClearColorRGBA, mClearColorRGBA);
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
     * @param deltaTime Time elapsed since last tick in seconds.
     * @return True if the renderer should continue running, false to exit.
     */
    bool Update(CommandList10& Cmdl, float deltaTime) const;

    void Resize(uint32_t Width, uint32_t Height);

    // Getters/Setters
    void SetClearColorRGBA(float r, float g, float b, float a) {
        mClearColorRGBA[0] = r;
        mClearColorRGBA[1] = g;
        mClearColorRGBA[2] = b;
        mClearColorRGBA[3] = a;
    }

    void SetModel(std::unique_ptr<MeshInstance>&& Model) {
        mModel = std::move(Model);
    }

   private:
    std::unique_ptr<MeshInstance> mModel;
    float mClearColorRGBA[4];

    D3D12_VIEWPORT mViewport;
    RECT mScissorRect;
};