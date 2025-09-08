#include "MainWindow.h"

#include "Graphics/Renderer.h"
#include "Logging/Logging.h"

bool MainWindow::Create(Renderer* Renderer, std::unique_ptr<MainWindow>& OutWindow) {
    // Handler to the module owning the window
    HMODULE hInstance = GetModuleHandle(nullptr);

    // Describing the main window with a window class
    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.hInstance = hInstance;
    wc.lpszClassName = MAIN_CLASS_NAME;

    // Windows event handler
    wc.lpfnWndProc = &MainWindow::WindowProc;

    // No menu
    wc.lpszMenuName = nullptr;

    // Load standard cursor
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);

    // No default background erase; children will paint
    wc.hbrBackground = nullptr;

    // Redraw on resize, own DC (meaning DX)
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;

    ATOM wcAtom = RegisterClassEx(&wc);
    if (wcAtom == 0) {
        LOG_ERROR(L"Failed to register window class. Error code: %d\n", GetLastError());
        return false;
    }

    // Creates a WindowState which decouples immediate window message processing from the graphics
    // handling

    // Create a local MainWindow instance to pass as lpParam,
    // passing nullptr as hWnd, we'll set it after CreateWindowEx
    std::unique_ptr<MainWindow> pWindow =
        std::make_unique<MainWindow>(nullptr, hInstance, wcAtom, Renderer);

    HWND hWnd = CreateWindowEx(
        // Extended styles
        WS_EX_OVERLAPPEDWINDOW | WS_EX_APPWINDOW,

        // Window class name
        MAIN_CLASS_NAME,

        // Window title
        L"DXStarter",

        // Window styles
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,

        // Window position
        CW_USEDEFAULT, CW_USEDEFAULT,

        // Window size
        DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT,

        // Parent window
        nullptr,

        // Menu handle
        nullptr,

        // Module instance
        hInstance,

        // Additional data; passing pointer as lpParam, see WM_NCCREATE message handling in
        // WindowProc
        pWindow.get());

    if (hWnd == nullptr) {
        LOG_ERROR(L"CreateWindowEx failed. Error: %ls", std::to_wstring(GetLastError()).c_str());
        return false;
    }

    // Update the hWnd in the MainWindow instance
    pWindow->mHWnd = hWnd;

    // Return the created window
    OutWindow = std::move(pWindow);
    return true;
}

// IMPORTANT! Requires MainWindow instance pointer to be set in the user data (GWLP_USERDATA)
// This function simply forwards the message to the instance handler.
LRESULT CALLBACK MainWindow::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    MainWindow* pThis;

    if (uMsg == WM_NCCREATE) {
        // If WM_NCCREATE get the 'this' pointer from lpCreateParams
        // and store it in user data.
        CREATESTRUCT* pCreateStruct = reinterpret_cast<CREATESTRUCT*>(lParam);
        pThis = reinterpret_cast<MainWindow*>(pCreateStruct->lpCreateParams);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
    } else {
        // Get the 'this' pointer from user data
        pThis = reinterpret_cast<MainWindow*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    }

    if (pThis) {
        return pThis->OnWindowMessage(hWnd, uMsg, wParam, lParam);
    }

    // Fallback
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

LRESULT MainWindow::OnWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            mRenderer->OnCreate(hWnd);
            return 0;
        }
        case WM_SIZE: {
            int width = LOWORD(lParam);
            int height = HIWORD(lParam);
            mRenderer->OnResize(width, height);
            return 0;
        }
        case WM_CLOSE: {
            // The user wants to close the window.
            DestroyWindow(hWnd);
            return 0;
        }
        case WM_DESTROY: {
            // The window is being destroyed. Post WM_QUIT to exit the application loop.
            PostQuitMessage(0);
            return 0;
        }
        default: {
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
        }
    }
}

int MainWindow::Run() {
    MSG msg{};

    // The main application loop
    do {
        // Windows message loop (user input)
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            if (msg.message == WM_QUIT) {
                mRenderer->Stop();
                break;
            }
        }

        // Update and render the app state
    } while (mRenderer->Update());

    // Exit
    LOG_INFO(L"Exiting application.\n");
    return msg.message == WM_QUIT ? static_cast<int>(msg.wParam) : 0;
}