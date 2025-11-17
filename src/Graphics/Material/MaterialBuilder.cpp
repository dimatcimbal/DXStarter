#include "MaterialBuilder.h"

#include "Graphics/Device.h"
#include "Logging/Logging.h"
#include "Material.h"

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

bool MaterialBuilder::CreateMaterial(Device& Device,
                                     RootSignature& RootSignature,
                                     std::shared_ptr<Material>& OutMaterial) {
    // Create Pipeline State object

    // RootSignature gets set in the Renderer.
    mPSODesc.pRootSignature = RootSignature.GetD3DRootSignature();

    // Input-assembler
    mPSODesc.InputLayout.NumElements = _countof(PositionOnly);
    mPSODesc.InputLayout.pInputElementDescs = PositionOnly;
    mPSODesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;

    // Vertex Shader
    // @see MaterialBuilder::SetVertexShaderBytecode

    // Hull Shader
    // @see MaterialBuilder::SetHullShaderBytecode

    // Domain Shader
    // @see MaterialBuilder::SetDomainShaderBytecode

    // Geometry Shader
    // @see MaterialBuilder::SetGeometryShaderBytecode

    // Rasterizer Primitive Topology setting
    mPSODesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    // Rasterizer Filling setting
    mPSODesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    // Rasterizer Culling settings
    mPSODesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;  // TODO: Enable culling when needed
    mPSODesc.RasterizerState.FrontCounterClockwise = FALSE;
    // Rasterizer Depth Bias settings
    mPSODesc.RasterizerState.DepthBias = 0;
    mPSODesc.RasterizerState.DepthBiasClamp = .0f;
    mPSODesc.RasterizerState.SlopeScaledDepthBias = .0f;
    // Rasterizer Depth Clipping setting
    mPSODesc.RasterizerState.DepthClipEnable = FALSE;
    // Rasterizer Multisampling setting
    mPSODesc.RasterizerState.MultisampleEnable = FALSE;
    // Rasterizer Antialiasing setting
    mPSODesc.RasterizerState.AntialiasedLineEnable = FALSE;
    // Rasterizer
    mPSODesc.RasterizerState.ForcedSampleCount = 0;
    mPSODesc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

    // Pixel Shader
    // @see MaterialBuilder::SetPixelShaderBytecode

    // Streaming Output Buffer
    mPSODesc.StreamOutput.NumEntries = 0;
    mPSODesc.StreamOutput.NumStrides = 0;
    mPSODesc.StreamOutput.pBufferStrides = nullptr;
    mPSODesc.StreamOutput.pSODeclaration = nullptr;
    mPSODesc.StreamOutput.RasterizedStream = 0;
    mPSODesc.NumRenderTargets = 1;  // Forward rendering
    mPSODesc.RTVFormats[0] = DXGI_FORMAT_DEFAULT_RTV;
    // All RTVFormats entries >= NumRenderTargets must be DXGI_FORMAT_UNKNOWN
    for (UINT i = 1; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i) {
        mPSODesc.RTVFormats[i] = DXGI_FORMAT_UNKNOWN;
    }
    mPSODesc.DSVFormat = DXGI_FORMAT_UNKNOWN;

    // Blending settings
    mPSODesc.BlendState.AlphaToCoverageEnable =
        FALSE;  // Not needed as no multisampling nor antialiasing are enabled
    mPSODesc.BlendState.IndependentBlendEnable = FALSE;

    mPSODesc.BlendState.RenderTarget[0].BlendEnable = FALSE;
    // Blending Logic Op
    mPSODesc.BlendState.RenderTarget[0].LogicOpEnable = FALSE;
    mPSODesc.BlendState.RenderTarget[0].LogicOp =
        D3D12_LOGIC_OP_NOOP;  // Logic op between source and destination

    //
    mPSODesc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_ZERO;
    mPSODesc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
    mPSODesc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;

    //
    mPSODesc.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ZERO;
    mPSODesc.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
    mPSODesc.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;

    mPSODesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    // Depth
    mPSODesc.DepthStencilState.DepthEnable = FALSE;
    mPSODesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    mPSODesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
    // Stencil
    mPSODesc.DepthStencilState.StencilEnable = FALSE;
    mPSODesc.DepthStencilState.StencilReadMask = 0;
    mPSODesc.DepthStencilState.StencilWriteMask = 0;
    // Front Face
    mPSODesc.DepthStencilState.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    mPSODesc.DepthStencilState.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
    mPSODesc.DepthStencilState.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
    mPSODesc.DepthStencilState.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
    // Back Face
    mPSODesc.DepthStencilState.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    mPSODesc.DepthStencilState.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
    mPSODesc.DepthStencilState.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
    mPSODesc.DepthStencilState.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
    // Sample
    mPSODesc.SampleMask = 0xFFFFFFFF;
    mPSODesc.SampleDesc.Count = 1;
    mPSODesc.SampleDesc.Quality = 0;

    mPSODesc.NodeMask = 0;
    mPSODesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

    // PSO Cache
    mPSODesc.CachedPSO.CachedBlobSizeInBytes = 0;
    mPSODesc.CachedPSO.pCachedBlob = nullptr;

    std::unique_ptr<PipelineState> pPipelineState;
    if (!Device.CreatePipelineState(mPSODesc, pPipelineState)) {
        LOG_ERROR(L"Failed to create pipeline state object.\n");
        return false;
    }

    if (!Material::Create(std::move(pPipelineState), OutMaterial)) {
        LOG_ERROR(L"Failed to create material object.\n");
        return false;
    }
    return true;
}
