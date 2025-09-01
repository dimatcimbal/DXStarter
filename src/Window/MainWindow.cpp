#include "MainWindow.h"

#include "Logging/Logging.h"

bool MainWindow::Create(std::unique_ptr<MainWindow>& OutWindow) {
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

    // Create the MainWindow instance before creating the window
    std::unique_ptr<GraphicsContext> pGraphicsContext;
    if (!GraphicsContext::Create(pGraphicsContext)) {
        LOG_ERROR(L"Failed to create GraphicsContext.\n");
        return false;
    }
    // Create a local MainWindow instance to pass as lpParam,
    // passing nullptr as hWnd, we'll set it after CreateWindowEx
    std::unique_ptr<MainWindow> pWindow =
        std::make_unique<MainWindow>(nullptr, hInstance, wcAtom, std::move(pGraphicsContext));

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
            // Window is being created
            if (!OnCreate(hWnd)) {
                LOG_ERROR(
                    L"Failed to initialize window, destroying window and exiting application.\n");
                PostQuitMessage(-1);
                return -1;
            }
            return 0;
        }
        case WM_SIZE: {
            int width = LOWORD(lParam);
            int height = HIWORD(lParam);
            if (!OnResize(width, height)) {
                LOG_ERROR(L"Failed to resize window, destroying window and exiting application.\n");
                DestroyWindow(hWnd);
                PostQuitMessage(-1);
                return -1;
            }
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

bool MainWindow::OnCreate(HWND hWnd) const {
    LOG_INFO(L"Creating main window.\n");

    if (!mGraphicsContext->OnWindowCreate(hWnd)) {
        LOG_ERROR(L"Failed while calling GraphicsContext::OnWindowCreated handler.\n");
        return false;
    }
    return true;
}

bool MainWindow::OnResize(int Width, int Height) const {
    if (Width == 0 || Height == 0) {
        // Window is minimized or has zero area; ignore resize
        // TODO: Pause rendering?
        return true;
    }

    // Checks negative values before int -> uint32_t conversion
    if (Width < 0 || Height < 0) {
        LOG_ERROR(L"Invalid window size: %d x %d\n", Width, Height);
        return false;
    }
    uint32_t NewWidth = static_cast<uint32_t>(Width);
    uint32_t NewHeight = static_cast<uint32_t>(Height);

    LOG_INFO(L"Resizing window to %u x %u\n", NewWidth, NewHeight);

    if (!mGraphicsContext->OnWindowResize(NewWidth, NewHeight)) {
        LOG_ERROR(L"Failed while calling GraphicsContext::OnWindowResize.\n");
        return false;
    }

    return true;
}

int MainWindow::Run() {
    MSG msg{};

    // The main application loop
    while (mIsMainLoopRunning) {
        // Windows message loop (user input)
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            if (msg.message == WM_QUIT) {
                mIsMainLoopRunning = false;
                break;
            }
        }

        // Scene update and the rendering part
        if (mIsMainLoopRunning) {
            // TODO: Scene update

            // Draw a frame
            if (!mGraphicsContext->Draw()) {
                LOG_ERROR(L"Failed to draw a frame.\n");
                mIsMainLoopRunning = false;
                break;
            }
        }
    }

    // Exit
    return msg.message == WM_QUIT ? static_cast<int>(msg.wParam) : 0;
}