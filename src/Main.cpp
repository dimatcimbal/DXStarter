#include <Windows.h>

#include <memory>

#include "Graphics/Device.h"
#include "Graphics/Renderer.h"
#include "Logging/Logging.h"
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
    std::unique_ptr<Device> pDevice;
    if (!Device::Create(GRAPHICS_FEATURE_LEVEL, true, true, pDevice)) {
        LOG_ERROR(L"Failed to create Device.\n");
        ShowErrorMessageBox();
        return -1;
    }

    // The Renderer
    std::unique_ptr<Renderer> pRenderer;
    if (!Renderer::Create(1024, pDevice.get(), pRenderer)) {
        LOG_ERROR(L"Failed to create Renderer.\n");
        ShowErrorMessageBox();
        return -1;
    }

    // A simple triangle
    float vertexData[] = {// A (x,y)
                          -1.f, -1.f,
                          // B (x,y)
                          0.f, 1.f,
                          // C (x,y)
                          1.f, -1.f};

    if (!pRenderer->LoadVertexData(vertexData, sizeof(vertexData))) {
        LOG_ERROR(L"Failed to load bytes into the renderer.\n");
        ShowErrorMessageBox();
        return -1;
    }

    // The main window.
    std::unique_ptr<MainWindow> mainWindow;
    if (!MainWindow::Create(pRenderer.get(), mainWindow)) {
        LOG_ERROR(L"Failed to create MainWindow.\n");
        ShowErrorMessageBox();
        return -1;
    }

    return mainWindow->Run();
}