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
        : mDevice(Device),
          mRenderer(Renderer),
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
    DXView(DXView&& other) noexcept
        : mDevice(other.mDevice),
          mRenderer(other.mRenderer),
          mSwapChain(std::exchange(other.mSwapChain, nullptr)),
          mWidth(std::exchange(other.mWidth, 0)),
          mHeight(std::exchange(other.mHeight, 0)),
          mIsRunning(std::exchange(other.mIsRunning, false)),
          mGraphicsHwnd(std::exchange(other.mGraphicsHwnd, nullptr)),
          mIsCreating(std::exchange(other.mIsCreating, false)),
          mNewWidth(std::exchange(other.mNewWidth, 0)),
          mNewHeight(std::exchange(other.mNewHeight, 0)),
          mIsMinimizing(std::exchange(other.mIsMinimizing, false)),
          mIsResizing(std::exchange(other.mIsResizing, false)),
          mLastFrameTime(std::exchange(other.mLastFrameTime, {})),
          mFrequency(std::exchange(other.mFrequency, {})) {}

    DXView& operator=(DXView&& other) noexcept {
        if (this != &other) {
            // References cannot be reassigned, so we keep the same references
            // This means the moved-from object will still reference the same Device/Renderer
            mSwapChain = std::exchange(other.mSwapChain, nullptr);
            mWidth = std::exchange(other.mWidth, 0);
            mHeight = std::exchange(other.mHeight, 0);
            mIsRunning = std::exchange(other.mIsRunning, false);
            mGraphicsHwnd = std::exchange(other.mGraphicsHwnd, nullptr);
            mIsCreating = std::exchange(other.mIsCreating, false);
            mNewWidth = std::exchange(other.mNewWidth, 0);
            mNewHeight = std::exchange(other.mNewHeight, 0);
            mIsMinimizing = std::exchange(other.mIsMinimizing, false);
            mIsResizing = std::exchange(other.mIsResizing, false);
            mLastFrameTime = std::exchange(other.mLastFrameTime, {});
            mFrequency = std::exchange(other.mFrequency, {});
        }
        return *this;
    }

    // Instance methods

    // Renderer state management methods below.
    void OnWindowCreate(HWND hWnd);
    void OnWindowResize(int NewWidth, int NewHeight);

    bool Update();

    void Stop() {
        mIsRunning = false;
    }

   private:
    Device& mDevice;
    Renderer& mRenderer;

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
