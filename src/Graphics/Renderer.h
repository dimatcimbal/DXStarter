#pragma once
#include <Windows.h>

#include "GraphicsContext.h"

class Renderer {
   public:
    Renderer(GraphicsContext* GraphicsContext) : mGraphicsContext{GraphicsContext} {}
    ~Renderer() = default;

    // Prohibit copying
    Renderer(const Renderer& copy) = delete;
    Renderer& operator=(const Renderer& copy) = delete;

    // Renderer state management methods below.

    void OnCreate(HWND hWnd) {
        LOG_INFO(L"Renderer::OnCreate with window handle %p\n", hWnd);
        mGraphicsHwnd = hWnd;
        mIsCreated = true;
    }

    void OnResize(int NewWidth, int NewHeight) {
        LOG_INFO(L"Renderer::OnResize to %d x %d\n", NewWidth, NewHeight);

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
    /**
     * Draws a frame.
     * @return
     */
    bool Draw() const;

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