#include <Windows.h>

#include <memory>

#include "Graphics/Device.h"
#include "Graphics/Renderer.h"
#include "IO/Bytes.h"
#include "IO/Paths.h"
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

    // A simple 3D triangle
    float VertexData[] = {// A (x,y,z)
                          -1.f, -1.f, 0.f,
                          // B (x,y,z)
                          0.f, 1.f, 0.f,
                          // C (x,y,z)
                          1.f, -1.f, 0.f};

    // DX context
    std::unique_ptr<Device> pDevice;
    if (!Device::Create(GRAPHICS_FEATURE_LEVEL, true, true, pDevice)) {
        LOG_ERROR(L"Failed to create Device.\n");
        ShowErrorMessageBox();
        return -1;
    }

    std::shared_ptr<Mesh> Tri;
    if (!pDevice->CreateMesh(3,                  // VertexCount
                             sizeof(float) * 3,  // VertexStrideInBytes (x, y, z)
                             VertexData,         // VertexData
                             Tri)) {
        LOG_ERROR(L"Failed to load create mesh.\n");
        ShowErrorMessageBox();
        return -1;
    }

    std::unique_ptr<MeshInstance> Model;
    if (!pDevice->CreateMeshInstance(Tri, Model)) {
        LOG_ERROR(L"Failed to create MeshInstance.\n");
        ShowErrorMessageBox();
        return -1;
    }

    // The Renderer
    std::unique_ptr<Renderer> pRenderer;
    if (!Renderer::Create(*pDevice, pRenderer)) {
        LOG_ERROR(L"Failed to create Renderer.\n");
        ShowErrorMessageBox();
        return -1;
    }

    pRenderer->SetModel(std::move(Model));

    // The main window.
    std::unique_ptr<MainWindow> pMainWindow;
    if (!MainWindow::Create(*pDevice, *pRenderer, pMainWindow)) {
        LOG_ERROR(L"Failed to create MainWindow.\n");
        ShowErrorMessageBox();
        return -1;
    }

    return pMainWindow->HandleMessages();
}