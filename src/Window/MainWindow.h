#pragma once

#include "Includes/WindowsIncl.h"
#include <memory>

// Main window class name for registration
const static LPCWSTR MAIN_CLASS_NAME = L"DXStarterMainWindow";

// Default window dimensions
static constexpr int DEFAULT_WINDOW_WIDTH = 1280;
static constexpr int DEFAULT_WINDOW_HEIGHT = 720;

class MainWindow {
public:
  /**
   * Factory method to create the main window.
   *
   * @param OutWindow A std::unique pointer that will hold the created
   * MainWindow instance.
   * @return true if the main window was successfully created and OutWindow is
   * set; false otherwise.
   */
  static bool Create(std::unique_ptr<MainWindow> &OutWindow);

  /**
   * Standard Windows message handler.
   *
   * @param hWnd    Handle to the window.
   * @param uMsg    Message identifier.
   * @param wParam  Additional message information (depends on message).
   * @param lParam  Additional message information (depends on message).
   * @return The result of message processing. The meaning of the return value
   * depends on the message sent.
   */
  static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam,
                                     LPARAM lParam);

  MainWindow(HWND hWnd, HMODULE hInstance, ATOM wcAtom)
      : mHWnd(hWnd), mHInstance(hInstance), mWcAtom(wcAtom) {

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
  MainWindow(const MainWindow &) = delete;
  MainWindow &operator=(const MainWindow &) = delete;

  int Run();

private:
  bool mIsMainLoopRunning{true};

  ATOM mWcAtom;
  HMODULE mHInstance;
  HWND mHWnd;
};
