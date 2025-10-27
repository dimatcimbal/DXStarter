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

    // A simple triangle
    float vertexData[] = {// A (x,y)
                          -1.f, -1.f,
                          // B (x,y)
                          0.f, 1.f,
                          // C (x,y)
                          1.f, -1.f};

    // DX context
    std::unique_ptr<Device> Device;
    if (!Device::Create(GRAPHICS_FEATURE_LEVEL, true, true, Device)) {
        LOG_ERROR(L"Failed to create Device.\n");
        ShowErrorMessageBox();
        return -1;
    }

    std::shared_ptr<Mesh> Tri;
    if (!Device->CreateMesh(sizeof(vertexData), vertexData, Tri)) {
        LOG_ERROR(L"Failed to load create mesh.\n");
        ShowErrorMessageBox();
        return -1;
    }

    std::unique_ptr<MeshInstance> Model;
    if (!Device->CreateMeshInstance(Tri, Model)) {
        LOG_ERROR(L"Failed to create MeshInstance.\n");
        ShowErrorMessageBox();
        return -1;
    }

    // The Renderer
    std::unique_ptr<Renderer> Renderer;
    if (!Renderer::Create(Device.get(), Renderer)) {
        LOG_ERROR(L"Failed to create Renderer.\n");
        ShowErrorMessageBox();
        return -1;
    }

    Renderer->SetModel(std::move(Model));

    // The main window.
    std::unique_ptr<MainWindow> MainWindow;
    if (!MainWindow::Create(Renderer.get(), MainWindow)) {
        LOG_ERROR(L"Failed to create MainWindow.\n");
        ShowErrorMessageBox();
        return -1;
    }

    return MainWindow->Run();
}