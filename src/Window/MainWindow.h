#pragma once

#include "Graphics/GraphicsContext.h"

#include <Windows.h>
#include <memory>

// Main window class name for registration
const static LPCWSTR MAIN_CLASS_NAME = L"DXStarterMainWindow";

// Default window dimensions
static constexpr int DEFAULT_WINDOW_WIDTH = 1280;
static constexpr int DEFAULT_WINDOW_HEIGHT = 720;

/**
 * MainWindow represents the main application window.
 * It handles window creation, message processing, and main application loop.
 */
class MainWindow {
  public:
    /**
     * Factory method to create the main application window.
     *
     * @param OutWindow A unique pointer to hold the created MainWindow instance.
     * @return true if the MainWindow was successfully created; false otherwise.
     */
    static bool Create(std::unique_ptr<MainWindow>& OutWindow);

    /**
     * Standard Windows message handler.
     *
     * @param hWnd    Handle to the window.
     * @param uMsg    Message identifier.
     * @param wParam  Additional message information (depends on message).
     * @param lParam  Additional message information (depends on message).
     * @return The result of message processing. The meaning of the return value depends on the
     * message sent.
     */
    static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    MainWindow(HWND hWnd,
               HMODULE hInstance,
               ATOM wcAtom,
               std::unique_ptr<GraphicsContext> pGraphicsContext)
        : mHWnd(hWnd), mHInstance(hInstance), mWcAtom(wcAtom),
          mGraphicsContext(std::move(pGraphicsContext)) {

        // Display and update the main window.
        ShowWindow(hWnd, SW_SHOWDEFAULT);
        UpdateWindow(hWnd);
    }

    ~MainWindow() {
        // Destroy the window
        if (mHWnd) {
            DestroyWindow(mHWnd);
            mHWnd = nullptr;
        }

        // Unregister the window class
        if (mWcAtom) {
            UnregisterClass(MAIN_CLASS_NAME, mHInstance);
            // Good practice to reset the value in the condition
            mWcAtom = 0;
        }
    }

    // No copy constructor or copy operator
    MainWindow(const MainWindow&) = delete;
    MainWindow& operator=(const MainWindow&) = delete;

    int Run();

  private:
    std::unique_ptr<GraphicsContext> mGraphicsContext;

    bool mIsMainLoopRunning{true};

    ATOM mWcAtom;
    HMODULE mHInstance;
    HWND mHWnd;
};
