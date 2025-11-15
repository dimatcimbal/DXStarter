#include "Material.h"

#include "Graphics/Device.h"
#include "IO/Bytes.h"
#include "IO/Paths.h"
#include "Logging/Logging.h"

D3D12_INPUT_ELEMENT_DESC PositionOnly[] = {
    // The semantics describing only position
    {
        "POSITION",                    // Semantic name
        0,                             // Semantic index (as there is only position element, set 0)
        DXGI_FORMAT_R32G32B32_FLOAT,   // Semantic format (x, y, z)
        0,                             // VertexBuffer view index to bind data from
        D3D12_APPEND_ALIGNED_ELEMENT,  // Offset within the vertex
        D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,  //
        0  // 0 due to D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA chosen above
    }};

bool Material::Create(Device& Device, std::shared_ptr<Material>& OutMaterial) {
    std::unique_ptr<Bytes> pVertexShaderBytecode;

    std::filesystem::path CompiledShaderDir;
    if (!Paths::GetMaterialsDirAbsPath(CompiledShaderDir)) {
        LOG_ERROR("Failed to get the path to the compiled shader directory.");
        return false;
    }

    if (!Bytes::Load(CompiledShaderDir / "Default.vertx.cso", pVertexShaderBytecode)) {
        LOG_ERROR("Failed to load vertex shader.");
        return false;
    }

    std::unique_ptr<Bytes> pPixelShaderBytecode;
    if (!Bytes::Load(CompiledShaderDir / "Default.pixel.cso", pPixelShaderBytecode)) {
        LOG_ERROR("Failed to load pixel shader.");
        return false;
    }

    std::unique_ptr<Bytes> pRootSigBytecode;
    if (!Bytes::Load(CompiledShaderDir / "Default.rsign.cso", pRootSigBytecode)) {
        LOG_ERROR("Failed to load root signature.");
        return false;
    }

    std::unique_ptr<RootSignature> pRootSignature;
    if (!Device.CreateRootSignature(*pRootSigBytecode, pRootSignature)) {
        LOG_ERROR("Failed to create root signature.");
        return false;
    }

    // Create Pipeline State object
    D3D12_GRAPHICS_PIPELINE_STATE_DESC PSODesc;
    PSODesc.pRootSignature = pRootSignature->GetD3DRootSignature();
    // Input-assembler
    PSODesc.InputLayout.NumElements = _countof(PositionOnly);
    PSODesc.InputLayout.pInputElementDescs = PositionOnly;
    PSODesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;

    // Vertex Shader
    PSODesc.VS.pShaderBytecode = pVertexShaderBytecode->GetBuffer();
    PSODesc.VS.BytecodeLength = pVertexShaderBytecode->GetSize();

    // Hull Shader
    PSODesc.HS.pShaderBytecode = nullptr;
    PSODesc.HS.BytecodeLength = 0;

    // Domain Shader
    PSODesc.DS.pShaderBytecode = nullptr;
    PSODesc.DS.BytecodeLength = 0;

    // Geometry Shader
    PSODesc.GS.pShaderBytecode = nullptr;
    PSODesc.GS.BytecodeLength = 0;

    // Rasterizer Primitive Topology setting
    PSODesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    // Rasterizer Filling setting
    PSODesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    // Rasterizer Culling settings
    PSODesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;  // TODO: Enable culling when needed
    PSODesc.RasterizerState.FrontCounterClockwise = FALSE;
    // Rasterizer Depth Bias settings
    PSODesc.RasterizerState.DepthBias = 0;
    PSODesc.RasterizerState.DepthBiasClamp = .0f;
    PSODesc.RasterizerState.SlopeScaledDepthBias = .0f;
    // Rasterizer Depth Clipping setting
    PSODesc.RasterizerState.DepthClipEnable = FALSE;
    // Rasterizer Multisampling setting
    PSODesc.RasterizerState.MultisampleEnable = FALSE;
    // Rasterizer Antialiasing setting
    PSODesc.RasterizerState.AntialiasedLineEnable = FALSE;
    // Rasterizer
    PSODesc.RasterizerState.ForcedSampleCount = 0;
    PSODesc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

    // Pixel Shader
    PSODesc.PS.pShaderBytecode = pPixelShaderBytecode->GetBuffer();
    PSODesc.PS.BytecodeLength = pPixelShaderBytecode->GetSize();

    // Streaming Output Buffer
    PSODesc.StreamOutput.NumEntries = 0;
    PSODesc.StreamOutput.NumStrides = 0;
    PSODesc.StreamOutput.pBufferStrides = nullptr;
    PSODesc.StreamOutput.pSODeclaration = nullptr;
    PSODesc.StreamOutput.RasterizedStream = 0;
    PSODesc.NumRenderTargets = 1;  // Forward rendering
    PSODesc.RTVFormats[0] = DXGI_FORMAT_DEFAULT_RTV;
    // All RTVFormats entries >= NumRenderTargets must be DXGI_FORMAT_UNKNOWN
    for (UINT i = 1; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i) {
        PSODesc.RTVFormats[i] = DXGI_FORMAT_UNKNOWN;
    }
    PSODesc.DSVFormat = DXGI_FORMAT_UNKNOWN;

    // Blending settings
    PSODesc.BlendState.AlphaToCoverageEnable =
        FALSE;  // Not needed as no multisampling nor antialiasing are enabled
    PSODesc.BlendState.IndependentBlendEnable = FALSE;

    PSODesc.BlendState.RenderTarget[0].BlendEnable = FALSE;
    // Blending Logic Op
    PSODesc.BlendState.RenderTarget[0].LogicOpEnable = FALSE;
    PSODesc.BlendState.RenderTarget[0].LogicOp =
        D3D12_LOGIC_OP_NOOP;  // Logic op between source and destination

    //
    PSODesc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_ZERO;
    PSODesc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
    PSODesc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;

    //
    PSODesc.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ZERO;
    PSODesc.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
    PSODesc.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;

    PSODesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    // Depth
    PSODesc.DepthStencilState.DepthEnable = FALSE;
    PSODesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    PSODesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
    // Stencil
    PSODesc.DepthStencilState.StencilEnable = FALSE;
    PSODesc.DepthStencilState.StencilReadMask = 0;
    PSODesc.DepthStencilState.StencilWriteMask = 0;
    // Front Face
    PSODesc.DepthStencilState.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    PSODesc.DepthStencilState.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
    PSODesc.DepthStencilState.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
    PSODesc.DepthStencilState.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
    // Back Face
    PSODesc.DepthStencilState.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    PSODesc.DepthStencilState.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
    PSODesc.DepthStencilState.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
    PSODesc.DepthStencilState.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
    // Sample
    PSODesc.SampleMask = 0xFFFFFFFF;
    PSODesc.SampleDesc.Count = 1;
    PSODesc.SampleDesc.Quality = 0;

    PSODesc.NodeMask = 0;
    PSODesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

    // PSO Cache
    PSODesc.CachedPSO.CachedBlobSizeInBytes = 0;
    PSODesc.CachedPSO.pCachedBlob = nullptr;

    std::unique_ptr<PipelineState> pPipelineState;
    if (!Device.CreatePipelineState(PSODesc, pPipelineState)) {
        LOG_ERROR(L"Failed to create pipeline state object.\n");
        return false;
    }

    OutMaterial = std::make_shared<Material>(std::move(pRootSignature), std::move(pPipelineState));
    return true;
}
