#include <Windows.h>

#include <filesystem>
#include <memory>

#include "Graphics/Device.h"
#include "Graphics/Material/Material.h"
#include "Graphics/Material/MaterialBuilder.h"
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

    // DX device
    std::unique_ptr<Device> pDevice;
    if (!Device::Create(GRAPHICS_FEATURE_LEVEL, true, true, pDevice)) {
        LOG_ERROR(L"Failed to create Device.\n");
        ShowErrorMessageBox();
        return -1;
    }

    // Load shader bytecode
    std::filesystem::path MaterialDir;
    if (!Paths::GetMaterialsDirAbsPath(MaterialDir)) {
        LOG_ERROR(L"Failed to get the path to the compiled shader directory.");
        ShowErrorMessageBox();
        return -1;
    }

    std::unique_ptr<Bytes> pVertexShaderBytecode;
    if (!Bytes::Load(MaterialDir / "Default.vertx.cso", pVertexShaderBytecode)) {
        LOG_ERROR(L"Failed to load vertex shader.");
        ShowErrorMessageBox();
        return -1;
    }

    std::unique_ptr<Bytes> pPixelShaderBytecode;
    if (!Bytes::Load(MaterialDir / "Default.pixel.cso", pPixelShaderBytecode)) {
        LOG_ERROR(L"Failed to load pixel shader.");
        ShowErrorMessageBox();
        return -1;
    }

    std::unique_ptr<Bytes> pRootSigBytecode;
    if (!Bytes::Load(MaterialDir / "Default.rsign.cso", pRootSigBytecode)) {
        LOG_ERROR(L"Failed to load root signature.");
        ShowErrorMessageBox();
        return -1;
    }

    // Create Material using MaterialBuilder
    MaterialBuilder builder;
    builder.SetVertexShaderBytecode(std::move(pVertexShaderBytecode))
           .SetPixelShaderBytecode(std::move(pPixelShaderBytecode))
           .SetRootSigBytecode(std::move(pRootSigBytecode));
    
    std::shared_ptr<Material> pMaterial;
    if (!builder.CreateMaterial(*pDevice, pMaterial)) {
        LOG_ERROR(L"Failed to create Material.\n");
        ShowErrorMessageBox();
        return -1;
    }

    // Create Geometry
    std::shared_ptr<Mesh> Tri;
    if (!pDevice->CreateMesh(3,                  // VertexCount
                             sizeof(float) * 3,  // VertexStrideInBytes (x, y, z)
                             VertexData,         // VertexData
                             Tri)) {
        LOG_ERROR(L"Failed to load create mesh.\n");
        ShowErrorMessageBox();
        return -1;
    }

    // Combine into a ModelInstance
    std::unique_ptr<MeshInstance> Model;
    if (!pDevice->CreateMeshInstance(Tri, pMaterial, Model)) {
        LOG_ERROR(L"Failed to create MeshInstance.\n");
        ShowErrorMessageBox();
        return -1;
    }

    // The Renderer
    std::unique_ptr<Renderer> pRenderer;
    if (!Renderer::Create(pRenderer)) {
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