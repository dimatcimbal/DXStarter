#pragma once
#include <Windows.h>

#include "Graphics/GraphicsContext.h"

class WindowState {
   public:
    WindowState(GraphicsContext* GraphicsContext) : mGraphicsContext(GraphicsContext) {}
    ~WindowState() = default;

    // Prohibit copying
    WindowState(const WindowState& copy) = delete;
    WindowState& operator=(const WindowState& copy) = delete;

    // Window message handler functions below

    void OnCreate(HWND hWnd) {
        LOG_INFO(L"WindowState::OnCreate with window handle %p\n", hWnd);
        mGraphicsHwnd = hWnd;
        mIsCreated = true;
    }

    void OnResize(int NewWidth, int NewHeight) {
        LOG_INFO(L"WindowState::OnResize to %d x %d\n", NewWidth, NewHeight);

        if (NewWidth == 0 || NewHeight == 0) {
            // Window is minimized or has zero area; ignore resize
            mIsMinimized = true;
            return;
        }

        mWidth = static_cast<uint32_t>(NewWidth);
        mHeight = static_cast<uint32_t>(NewHeight);

        mIsMinimized = false;
        mIsResized = true;
    }

    void Stop() {
        mIsRunning = false;
    }

    // Frame update function
    bool Update();

   private:
    // Not owned by this class
    GraphicsContext* mGraphicsContext;

    // Main loop control
    bool mIsRunning{true};

    // OnCreate args
    HWND mGraphicsHwnd{nullptr};
    bool mIsCreated{false};

    // OnResize args
    bool mIsMinimized{false};
    bool mIsResized{false};
    uint32_t mWidth{0};
    uint32_t mHeight{0};
};