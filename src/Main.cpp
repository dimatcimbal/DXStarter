#include <Windows.h>

#include <memory>

#include "Window/MainWindow.h"
#include "Window/WindowState.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    UNREFERENCED_PARAMETER(hInstance);
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nCmdShow);

    // DX context
    std::unique_ptr<GraphicsContext> pGraphicsContext;
    if (!GraphicsContext::Create(pGraphicsContext)) {
        MessageBox(nullptr, L"Failed to create GraphicsContext.\n", L"Error", MB_OK | MB_ICONERROR);
        return -1;
    }

    // The main window.
    std::unique_ptr<MainWindow> mainWindow;
    if (!MainWindow::Create(pGraphicsContext.get(), mainWindow)) {
        MessageBox(nullptr, L"Failed to create main window", L"Error", MB_OK | MB_ICONERROR);
        return -1;
    }

    return mainWindow->Run();
}