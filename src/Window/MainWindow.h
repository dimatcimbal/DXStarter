#pragma once

#include <Windows.h>

#include <memory>

#include "DXView.h"
#include "Graphics/Renderer.h"

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
    static void ShowErrorMessageBox(const wchar_t* Err = L"Failed to start the application.") {
        MessageBox(nullptr, Err, L"Error", MB_OK | MB_ICONERROR);
    }

    /**
     * Factory method to create the main application window.
     * @param Device Reference to the Device instance.
     * @param Renderer Reference to the Renderer instance.
     * @param OutWindow A unique pointer to hold the created MainWindow instance.
     * @return true if the MainWindow was successfully created; false otherwise.
     */
    static bool Create(Device& Device, Renderer& Renderer, std::unique_ptr<MainWindow>& OutWindow);

    /**
     * Standard Windows message handler.
     *
     * @param HWnd    Handle to the window.
     * @param UMsg    Message identifier.
     * @param WParam  Additional message information (depends on message).
     * @param LParam  Additional message information (depends on message).
     * @return The result of message processing. The meaning of the return value depends on the
     * message sent.
     */
    static LRESULT CALLBACK WindowProc(HWND HWnd, UINT UMsg, WPARAM WParam, LPARAM LParam);

    MainWindow(HWND HWnd, HMODULE HInstance, ATOM WcAtom, std::unique_ptr<DXView>&& DXView)
        : mHWnd(HWnd), mHInstance(HInstance), mWcAtom(WcAtom), mDXView(std::move(DXView)) {
        // Display and update the main window.
        ShowWindow(HWnd, SW_SHOWDEFAULT);
        UpdateWindow(HWnd);
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

    // Provides MainWindow instance context (pThis) to the static WindowProc.
    LRESULT OnWindowMessage(HWND HWnd, UINT UMsg, WPARAM WParam, LPARAM LParam);

    // The main loop
    int HandleMessages();

   private:
    std::unique_ptr<DXView> mDXView;

    HMODULE mHInstance;
    ATOM mWcAtom;
    HWND mHWnd;
};
