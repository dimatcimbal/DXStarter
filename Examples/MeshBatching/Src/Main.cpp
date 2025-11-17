// Examples/MeshBatching
#include <Windows.h>

#include <filesystem>
#include <memory>
#include <random>

#include "Graphics/Device.h"
#include "Graphics/Material/Material.h"
#include "Graphics/Material/MaterialBuilder.h"
#include "Graphics/Mesh/Mesh.h"
#include "Graphics/Renderer.h"
#include "Graphics/Scene/Node.h"
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

    // Load shader bytecode directory
    std::filesystem::path materialDir;
    if (!Paths::GetMaterialsDir(materialDir)) {
        LOG_ERROR(L"Failed to get the path to the compiled shader directory.");
        MainWindow::ShowErrorMessageBox();
        return -1;
    }

    // Load Red material shaders
    std::unique_ptr<ByteBuffer> vertexShaderBytecode;
    if (!ByteBuffer::Create(materialDir / "WorldPosition.vertx.cso", vertexShaderBytecode)) {
        LOG_ERROR(L"Failed to load Red vertex shader.");
        MainWindow::ShowErrorMessageBox();
        return -1;
    }

    std::unique_ptr<ByteBuffer> redPixelShaderBytecode;
    if (!ByteBuffer::Create(materialDir / "ColorRed.pixel.cso", redPixelShaderBytecode)) {
        LOG_ERROR(L"Failed to load Red pixel shader.");
        MainWindow::ShowErrorMessageBox();
        return -1;
    }

    // Load Blue material shader
    std::unique_ptr<ByteBuffer> bluePixelShaderBytecode;
    if (!ByteBuffer::Create(materialDir / "ColorBlue.pixel.cso", bluePixelShaderBytecode)) {
        LOG_ERROR(L"Failed to load Blue pixel shader.");
        MainWindow::ShowErrorMessageBox();
        return -1;
    }

    // DX device
    std::unique_ptr<Device> device;
    if (!Device::Create(GRAPHICS_FEATURE_LEVEL, true, true, device)) {
        LOG_ERROR(L"Failed to create Device.\n");
        MainWindow::ShowErrorMessageBox();
        return -1;
    }

    std::unique_ptr<ByteBuffer> rootSignBytecode;
    if (!ByteBuffer::Create(materialDir / "WorldPosition.rsign.cso", rootSignBytecode)) {
        LOG_ERROR(L"Failed to load Red root signature.");
        MainWindow::ShowErrorMessageBox();
        return -1;
    }

    std::unique_ptr<RootSignature> rootSignature;
    if (!device->CreateRootSignature(*rootSignBytecode, rootSignature)) {
        LOG_ERROR(L"Failed to create root signature.");
        return false;
    }

    // The Renderer (use Red root signature)
    std::unique_ptr<Renderer> renderer;
    if (!Renderer::Create(*rootSignature, renderer)) {
        LOG_ERROR(L"Failed to create Renderer.\n");
        MainWindow::ShowErrorMessageBox();
        return -1;
    }

    // Create Red Material
    MaterialBuilder redMaterialBuilder;
    std::shared_ptr<Material> redMaterial;
    if (!redMaterialBuilder.SetVertexShaderBytecode(*vertexShaderBytecode)
             .SetPixelShaderBytecode(*redPixelShaderBytecode)
             .CreateMaterial(*device, *rootSignature, redMaterial)) {
        LOG_ERROR(L"Failed to create Red Material.\n");
        MainWindow::ShowErrorMessageBox();
        return -1;
    }
    MaterialId redMaterialId(redMaterial->GetMaterialId());

    // Create Blue Material
    MaterialBuilder blueMaterialBuilder;
    std::shared_ptr<Material> blueMaterial;
    if (!blueMaterialBuilder.SetVertexShaderBytecode(*vertexShaderBytecode)
             .SetPixelShaderBytecode(*bluePixelShaderBytecode)
             .CreateMaterial(*device, *rootSignature, blueMaterial)) {
        LOG_ERROR(L"Failed to create Blue Material.\n");
        MainWindow::ShowErrorMessageBox();
        return -1;
    }
    MaterialId blueMaterialId(blueMaterial->GetMaterialId());

    // Create shared Geometry (all triangles use the same mesh)
    std::unique_ptr<Mesh> tri;
    if (!device->CreateMesh(3,                  // VertexCount
                            sizeof(float) * 3,  // VertexStrideInBytes (x, y, z)
                            VertexData,         // VertexData
                            tri)) {
        LOG_ERROR(L"Failed to create mesh.\n");
        MainWindow::ShowErrorMessageBox();
        return -1;
    }

    // Create multiple triangles (few tens = ~30 triangles)
    const int numTriangles = 30;
    std::unique_ptr<Node> rootNode = std::make_unique<Node>();

    // Random number generator for positioning and rotating
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> posDist(-1.0f, 1.0f);

    for (int i = 0; i < numTriangles; ++i) {
        // Alternate between Red and Blue materials
        MaterialId materialId = (i % 2 == 0) ? redMaterialId : blueMaterialId;

        // Create Node for this triangle
        std::unique_ptr<Node> node;
        if (!device->CreateMeshNode(materialId, *tri, node)) {
            LOG_ERROR(L"Failed to create Node.\n");
            MainWindow::ShowErrorMessageBox();
            return -1;
        }

        // Position triangle randomly (from -1.0f to 1.0f)
        node->GetTransform().Translate(Vector3(posDist(gen), posDist(gen), 0));

        rootNode->AddChild(std::move(node));
    }

    renderer->SetScene(*rootNode);

    // The main window.
    std::unique_ptr<MainWindow> mainWindow;
    if (!MainWindow::Create(*device, *renderer, mainWindow)) {
        LOG_ERROR(L"Failed to create MainWindow.\n");
        MainWindow::ShowErrorMessageBox();
        return -1;
    }

    return mainWindow->HandleMessages();
}
