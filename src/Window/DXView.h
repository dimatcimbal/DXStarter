#pragma once

#include <Windows.h>

#include <memory>
#include <utility>

#include "Graphics/SwapChain.h"

// Forward declarations
class Device;
class Renderer;

/**
 * High-level view class. Manages swap chain, scene update and draw calls.
 */
class DXView {
   public:
    DXView(Device& Device, Renderer& Renderer)
        : mDevice(&Device),
          mRenderer(&Renderer),
          mWidth(0),
          mHeight(0),
          mNewWidth(0),
          mNewHeight(0),
          mGraphicsHwnd(nullptr),
          mIsCreating(false),
          mIsMinimizing(false),
          mIsResizing(false),
          mIsRunning(false),
          mFrequency({}),
          mLastFrameTime({}) {
        QueryPerformanceFrequency(&mFrequency);
    }

    ~DXView() = default;

    // Prohibit copying
    DXView(DXView const&) = delete;
    DXView& operator=(DXView const&) = delete;

    // Allow moving
    DXView(DXView&& Other) noexcept
        : mDevice(std::exchange(Other.mDevice, nullptr)),
          mRenderer(std::exchange(Other.mRenderer, nullptr)),
          mSwapChain(std::exchange(Other.mSwapChain, nullptr)),
          mWidth(std::exchange(Other.mWidth, 0)),
          mHeight(std::exchange(Other.mHeight, 0)),
          mIsRunning(std::exchange(Other.mIsRunning, false)),
          mGraphicsHwnd(std::exchange(Other.mGraphicsHwnd, nullptr)),
          mIsCreating(std::exchange(Other.mIsCreating, false)),
          mNewWidth(std::exchange(Other.mNewWidth, 0)),
          mNewHeight(std::exchange(Other.mNewHeight, 0)),
          mIsMinimizing(std::exchange(Other.mIsMinimizing, false)),
          mIsResizing(std::exchange(Other.mIsResizing, false)),
          mLastFrameTime(std::exchange(Other.mLastFrameTime, {})),
          mFrequency(std::exchange(Other.mFrequency, {})) {}

    DXView& operator=(DXView&& Other) noexcept {
        if (this != &Other) {
            mDevice = std::exchange(Other.mDevice, nullptr);
            mRenderer = std::exchange(Other.mRenderer, nullptr);
            mSwapChain = std::exchange(Other.mSwapChain, nullptr);
            mWidth = std::exchange(Other.mWidth, 0);
            mHeight = std::exchange(Other.mHeight, 0);
            mIsRunning = std::exchange(Other.mIsRunning, false);
            mGraphicsHwnd = std::exchange(Other.mGraphicsHwnd, nullptr);
            mIsCreating = std::exchange(Other.mIsCreating, false);
            mNewWidth = std::exchange(Other.mNewWidth, 0);
            mNewHeight = std::exchange(Other.mNewHeight, 0);
            mIsMinimizing = std::exchange(Other.mIsMinimizing, false);
            mIsResizing = std::exchange(Other.mIsResizing, false);
            mLastFrameTime = std::exchange(Other.mLastFrameTime, {});
            mFrequency = std::exchange(Other.mFrequency, {});
        }
        return *this;
    }

    // Instance methods

    // Renderer state management methods below.
    void OnWindowCreate(HWND HWnd);
    void OnWindowResize(int NewWidth, int NewHeight);

    bool Update();

    void Stop() {
        mIsRunning = false;
    }

   private:
    Device* mDevice;
    Renderer* mRenderer;

    // Owned resources
    std::unique_ptr<SwapChain> mSwapChain;

    // Cached window dimensions
    uint32_t mWidth;
    uint32_t mHeight;

    // Main loop control
    bool mIsRunning;

    // OnCreate args
    HWND mGraphicsHwnd;
    bool mIsCreating;

    // OnResize args
    uint32_t mNewWidth;
    uint32_t mNewHeight;
    bool mIsMinimizing;
    bool mIsResizing;

    // Timing variables for delta time calculation
    LARGE_INTEGER mLastFrameTime;
    LARGE_INTEGER mFrequency;
};
