#pragma once

#include <Windows.h>

#include <memory>

#include "Logging/Logging.h"
#include "Mesh/Mesh.h"
#include "Mesh/MeshInstance.h"
#include "Resources/ByteBuffer.h"
#include "Resources/UploadBuffer.h"
#include "SwapChain.h"

// Forward declarations
class Device;

/**
 * High-level renderer class. Manages swap chain, render targets, and draw calls.
 */
class Renderer {
   public:
    static bool Create(Device* Device, std::unique_ptr<Renderer>& OutRenderer);

    Renderer(Device* Device) : mDevice{Device} {}

    ~Renderer() {
        LOG_INFO(L"\tFreeing Renderer.\n");
    }

    // Prohibit copying
    Renderer(const Renderer& copy) = delete;
    Renderer& operator=(const Renderer& copy) = delete;

    // Renderer state management methods below.
    void OnWindowCreate(HWND hWnd);
    void OnWindowResize(int NewWidth, int NewHeight);

    void SetModel(std::unique_ptr<MeshInstance>&& Model) {
        mModel = std::move(Model);
    }

    /*
     * Frame update function
     */
    bool Update();

    void Stop() {
        mIsRunning = false;
    }

   private:
    /**
     * Draws a frame.
     * @return
     */
    bool Draw() const;

    /**
     * Main loop tick function.
     * @param deltaTime Time elapsed since last tick in seconds.
     * @return True if the renderer should continue running, false to exit.
     */
    bool Tick(float deltaTime) const;

    bool FlushAll() const;

   private:
    // Not owned by this class
    Device* mDevice;
    std::unique_ptr<MeshInstance> mModel;

    // Owned resource
    std::unique_ptr<SwapChain> mSwapChain;
    std::unique_ptr<UploadBuffer> mUploadBuffer;
    std::unique_ptr<ByteBuffer> mVertexBuffer;

    // Cached window dimensions
    uint32_t mWidth{0};
    uint32_t mHeight{0};

    // Main loop control
    bool mIsRunning{true};

    // OnCreate args
    HWND mGraphicsHwnd{nullptr};
    bool mIsCreated{false};

    // OnResize args
    bool mIsMinimized{false};
    bool mIsResized{false};
    uint32_t mNewWidth{0};
    uint32_t mNewHeight{0};

    // Timing variables for delta time calculation
    LARGE_INTEGER mLastFrameTime{};
    LARGE_INTEGER mFrequency{};
    bool mFirstFrame{true};
};