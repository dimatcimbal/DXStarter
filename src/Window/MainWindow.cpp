#include "MainWindow.h"

#include "Graphics/Device.h"
#include "Logging/Logging.h"

bool MainWindow::Create(Device& Device,
                        Renderer& Renderer,
                        std::unique_ptr<MainWindow>& OutWindow) {
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
    std::unique_ptr<DXView> pDXView = std::make_unique<DXView>(Device, Renderer);

    // Create a local MainWindow instance to pass as lpParam,
    // passing nullptr as HWnd, we'll set it after CreateWindowEx
    std::unique_ptr<MainWindow> pWindow =
        std::make_unique<MainWindow>(nullptr, hInstance, wcAtom, std::move(pDXView));

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
        LOG_ERROR(L"CreateWindowEx failed. Error: %d", GetLastError());
        return false;
    }

    // Update the HWnd in the MainWindow instance
    pWindow->mHWnd = hWnd;

    // Return the created window
    OutWindow = std::move(pWindow);
    return true;
}

// IMPORTANT! Requires MainWindow instance pointer to be set in the user data (GWLP_USERDATA)
// This function simply forwards the message to the instance handler.
LRESULT CALLBACK MainWindow::WindowProc(HWND HWnd, UINT UMsg, WPARAM WParam, LPARAM LParam) {
    MainWindow* pThis;

    if (UMsg == WM_NCCREATE) {
        // If WM_NCCREATE get the 'this' pointer from lpCreateParams
        // and store it in user data.
        CREATESTRUCT* pCreateStruct = reinterpret_cast<CREATESTRUCT*>(LParam);
        pThis = reinterpret_cast<MainWindow*>(pCreateStruct->lpCreateParams);

        // Updating window storage, setting user data in this case hence GWLP_USERDATA
        SetWindowLongPtr(HWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
    } else {
        // Get the 'this' pointer from user data
        pThis = reinterpret_cast<MainWindow*>(GetWindowLongPtr(HWnd, GWLP_USERDATA));
    }

    if (pThis) {
        return pThis->OnWindowMessage(HWnd, UMsg, WParam, LParam);
    }

    // Fallback
    return DefWindowProc(HWnd, UMsg, WParam, LParam);
}

LRESULT MainWindow::OnWindowMessage(HWND HWnd, UINT UMsg, WPARAM WParam, LPARAM LParam) {
    switch (UMsg) {
        case WM_CREATE: {
            mDXView->OnWindowCreate(HWnd);
            return 0;
        }
        case WM_SIZE: {
            int width = LOWORD(LParam);
            int height = HIWORD(LParam);
            mDXView->OnWindowResize(width, height);
            return 0;
        }
        case WM_CLOSE: {
            // The user wants to close the window.
            DestroyWindow(HWnd);
            return 0;
        }
        case WM_DESTROY: {
            // The window is being destroyed. Post WM_QUIT to exit the application loop.
            PostQuitMessage(0);
            return 0;
        }
        default: {
            return DefWindowProc(HWnd, UMsg, WParam, LParam);
        }
    }
}

int MainWindow::HandleMessages() {
    MSG msg{};

    // The main application loop
    do {
        // Windows message loop (user input)
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            if (msg.message == WM_QUIT) {
                mDXView->Stop();
                break;
            }
        }

        // Update and render the app state
    } while (mDXView->Update());

    // Exit
    LOG_INFO(L"Exiting application.\n");
    return msg.message == WM_QUIT ? static_cast<int>(msg.wParam) : 0;
}