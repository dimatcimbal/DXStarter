#include <Windows.h>

#include <memory>

#include "Window/MainWindow.h"

static void ShowErrorMessageBox(const wchar_t* err = L"Failed to start the application.") {
    MessageBox(nullptr, err, L"Error", MB_OK | MB_ICONERROR);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    UNREFERENCED_PARAMETER(hInstance);
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nCmdShow);

    // DX context
    std::unique_ptr<GraphicsContext> pGraphicsContext;
    if (!GraphicsContext::Create(pGraphicsContext)) {
        LOG_ERROR(L"Failed to create GraphicsContext.\n");
        ShowErrorMessageBox();
        return -1;
    }

    // The Renderer
    auto pRenderer = std::make_unique<Renderer>(pGraphicsContext.get());

    // The main window.
    std::unique_ptr<MainWindow> mainWindow;
    if (!MainWindow::Create(pRenderer.get(), mainWindow)) {
        LOG_ERROR(L"Failed to create MainWindow.\n");
        ShowErrorMessageBox();
        return -1;
    }

    return mainWindow->Run();
}