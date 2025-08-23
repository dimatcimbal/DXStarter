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
        LOG_ERROR(L"Failed to register window class as %ls",
                  std::to_wstring(GetLastError()).c_str());
        return false;
    }

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

        // Additional data
        nullptr);

    if (hWnd == nullptr) {
        LOG_ERROR(L"CreateWindowEx failed. Error: %ls", std::to_wstring(GetLastError()).c_str());
        return false;
    }

    std::unique_ptr<GraphicsContext> OutGraphicsContext;
    if (!GraphicsContext::Create(OutGraphicsContext)) {
        LOG_ERROR(L"Failed to create GraphicsContext.\n");
        return false;
    }

    OutWindow =
        std::make_unique<MainWindow>(hWnd, hInstance, wcAtom, std::move(OutGraphicsContext));
    return true;
}

LRESULT CALLBACK MainWindow::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

    switch (uMsg) {
    case WM_CLOSE:
        // The user wants to close the window.
        DestroyWindow(hWnd);
        return 0;

    case WM_DESTROY:
        // The window is being destroyed. Post WM_QUIT to exit the application loop.
        PostQuitMessage(0);
        return 0;

    default:
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
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
            // TODO: Rendering
        }
    }

    // Exit
    return msg.message == WM_QUIT ? static_cast<int>(msg.wParam) : 0;
}