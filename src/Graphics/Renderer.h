#pragma once

#include <Windows.h>

#include <memory>

#include "Logging/Logging.h"
#include "Resources/DefaultBuffer.h"
#include "Resources/UploadBuffer.h"
#include "SwapChain.h"

// Forward declarations
class Device;

/**
 * High-level renderer class. Manages swap chain, render targets, and draw calls.
 */
class Renderer {
   public:
    static bool Create(size_t VertexBufferSize,
                       Device* pDevice,
                       std::unique_ptr<Renderer>& OutRenderer);

    Renderer(Device* Device,
             std::unique_ptr<DefaultBuffer> mVertexBuffer,
             std::unique_ptr<UploadBuffer> mUploadBuffer)
        : mDevice{Device},
          mVertexBuffer{std::move(mVertexBuffer)},
          mUploadBuffer{std::move(mUploadBuffer)} {}

    ~Renderer() {
        LOG_INFO(L"\tFreeing Renderer.\n");
    }

    // Prohibit copying
    Renderer(const Renderer& copy) = delete;
    Renderer& operator=(const Renderer& copy) = delete;

    // Renderer state management methods below.
    void OnWindowCreate(HWND hWnd);
    void OnWindowResize(int NewWidth, int NewHeight);

    bool LoadVertexData(const void* data, size_t size) const;

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

    bool FlushAll() const;

   private:
    // Not owned by this class
    Device* mDevice;

    // Owned resource
    std::unique_ptr<SwapChain> mSwapChain;
    std::unique_ptr<UploadBuffer> mUploadBuffer;
    std::unique_ptr<DefaultBuffer> mVertexBuffer;

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
};