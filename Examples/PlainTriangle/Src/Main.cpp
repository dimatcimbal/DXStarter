// Examples/PlainTriangle
#include <Windows.h>

#include <filesystem>
#include <memory>

#include "Graphics/Device.h"
#include "Graphics/Material/Material.h"
#include "Graphics/Material/MaterialBuilder.h"
#include "Graphics/Renderer.h"
#include "Scene/Node.h"
#include "IO/ByteBuffer.h"
#include "IO/Paths.h"
#include "Logging/Logging.h"
#include "Window/MainWindow.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    UNREFERENCED_PARAMETER(hInstance);
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nCmdShow);

    // A simple 3D triangle
    float VertexData[] = {// A (x,y,z)
                          -0.1f, -0.1f, 0.f,
                          // B (x,y,z)
                          0.f, 0.1f, 0.f,
                          // C (x,y,z)
                          0.1f, -0.1f, 0.f};
    // DX device
    std::unique_ptr<Device> device;
    if (!Device::Create(GRAPHICS_FEATURE_LEVEL, true, true, device)) {
        LOG_ERROR(L"Failed to create Device.\n");
        MainWindow::ShowErrorMessageBox();
        return -1;
    }

    // Load shader bytecode
    std::filesystem::path materialDir;
    if (!Paths::GetMaterialsDir(materialDir)) {
        LOG_ERROR(L"Failed to get the path to the compiled shader directory.");
        MainWindow::ShowErrorMessageBox();
        return -1;
    }

    std::unique_ptr<ByteBuffer> vertexShaderBytecode;
    if (!ByteBuffer::Create(materialDir / "LocalPosition.vertx.cso", vertexShaderBytecode)) {
        LOG_ERROR(L"Failed to load vertex shader.");
        MainWindow::ShowErrorMessageBox();
        return -1;
    }

    std::unique_ptr<ByteBuffer> pixelShaderBytecode;
    if (!ByteBuffer::Create(materialDir / "ColorRed.pixel.cso", pixelShaderBytecode)) {
        LOG_ERROR(L"Failed to load pixel shader.");
        MainWindow::ShowErrorMessageBox();
        return -1;
    }

    std::unique_ptr<ByteBuffer> rootSignBytecode;
    if (!ByteBuffer::Create(materialDir / "Base.rsign.cso", rootSignBytecode)) {
        LOG_ERROR(L"Failed to load root signature.");
        MainWindow::ShowErrorMessageBox();
        return -1;
    }

    std::unique_ptr<RootSignature> rootSignature;
    if (!device->CreateRootSignature(*rootSignBytecode, rootSignature)) {
        LOG_ERROR(L"Failed to create root signature.");
        return -1;
    }

    // Create Material using MaterialBuilder; Returns a shared_ptr as Material class caches all
    // materials
    MaterialBuilder materialBuilder;
    std::shared_ptr<Material> material;
    if (!materialBuilder.SetVertexShaderBytecode(*vertexShaderBytecode)
             .SetPixelShaderBytecode(*pixelShaderBytecode)
             .CreateMaterial(*device, *rootSignature, material)) {
        LOG_ERROR(L"Failed to create Material.\n");
        MainWindow::ShowErrorMessageBox();
        return -1;
    }

    MaterialId materialId(material->GetMaterialId());

    // Create Geometry
    std::unique_ptr<Mesh> tri;
    if (!device->CreateMesh(3,                  // VertexCount
                            sizeof(float) * 3,  // VertexStrideInBytes (x, y, z)
                            VertexData,         // VertexData
                            tri)) {
        LOG_ERROR(L"Failed to create mesh.\n");
        MainWindow::ShowErrorMessageBox();
        return -1;
    }

    std::unique_ptr<Node> scene;
    if (!device->CreateMeshNode(materialId, *tri, scene)) {
        LOG_ERROR(L"Failed to create Node.\n");
        MainWindow::ShowErrorMessageBox();
        return -1;
    }

    // The Renderer
    std::unique_ptr<Renderer> renderer;
    if (!Renderer::Create(*rootSignature, renderer)) {
        LOG_ERROR(L"Failed to create Renderer.\n");
        MainWindow::ShowErrorMessageBox();
        return -1;
    }

    renderer->SetScene(*scene);

    // The main window.
    std::unique_ptr<MainWindow> mainWindow;
    if (!MainWindow::Create(*device, *renderer, mainWindow)) {
        LOG_ERROR(L"Failed to create MainWindow.\n");
        MainWindow::ShowErrorMessageBox();
        return -1;
    }

    return mainWindow->HandleMessages();
}