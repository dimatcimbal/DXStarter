#pragma once
#include <Windows.h>

#include "Device.h"

class Renderer {
   public:
    Renderer(Device* Device) : mDevice{Device} {}
    ~Renderer() {
        LOG_INFO(L"\tFreeing Renderer.\n");
    }

    // Prohibit copying
    Renderer(const Renderer& copy) = delete;
    Renderer& operator=(const Renderer& copy) = delete;

    // Renderer state management methods below.
    void OnCreate(HWND hWnd);
    void OnResize(int NewWidth, int NewHeight);
    void Stop() {
        mIsRunning = false;
    }

    // Frame update function
    bool Update();

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