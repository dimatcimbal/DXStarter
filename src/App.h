#pragma once
#include <memory>

#include "Graphics/GraphicsContext.h"

class App {
   public:
    static bool Create(App*& OutApp);

    // Prohibit copying
    App(App& copy) = delete;
    App& operator=(const App& copy) = delete;

    // Event handles functions
    void OnCreate(HWND hWnd) {
        LOG_INFO(L"App::OnCreate with window handle %p\n", hWnd);
        mGraphicsHwnd = hWnd;
        mIsCreated = true;
    }

    void OnResize(int NewWidth, int NewHeight) {
        LOG_INFO(L"App::OnResize to %d x %d\n", NewWidth, NewHeight);

        if (NewWidth == 0 || NewHeight == 0) {
            // Window is minimized or has zero area; ignore resize
            mIsMinimized = true;
            return;
        }

        mWigth = static_cast<uint32_t>(NewWidth);
        mHeight = static_cast<uint32_t>(NewHeight);

        mIsMinimized = false;
        mIsResized = true;
    }

    void Stop() {
        mIsRunning = false;
    }
    bool Update();

   private:
    App(std::unique_ptr<GraphicsContext>&& GraphicsContext)
        : mGraphicsContext(std::move(GraphicsContext)) {}

    std::unique_ptr<GraphicsContext> mGraphicsContext;

    // Main loop control
    bool mIsRunning{true};

    // OnCreate args
    HWND mGraphicsHwnd{nullptr};
    bool mIsCreated{false};

    // OnResize args
    bool mIsMinimized{false};
    bool mIsResized{false};
    uint32_t mWigth{0};
    uint32_t mHeight{0};
};