#include "Device.h"

#include "CommandList10.h"
#include "IO/ByteBuffer.h"
#include "Logging/Logging.h"
#include "Mesh/MeshInstance.h"
#include "Resource/UploadBuffer.h"
#include "RootSignature.h"
#include "Scene/Node.h"

using Microsoft::WRL::ComPtr;

bool Device::Create(D3D_FEATURE_LEVEL FeatureLevel,
                    bool IsHardwareDevice,
                    bool HasMaxVideoMemory,
                    std::unique_ptr<Device>& OutDevice) {
    std::unique_ptr<DebugLayer> debugLayer;
    if (!DebugLayer::Create(debugLayer)) {
        LOG_ERROR(L"Failed to initialize the Debug Layer.\n");
        return false;
    }

    ComPtr<IDXGIFactory7> dxgiFactory;
    if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)))) {
        LOG_ERROR(L"Failed to create DXGI factory.\n");
        return false;
    }

    ComPtr<ID3D12Device14> d3dDevice;
    if (!GetBestDevice(dxgiFactory, FeatureLevel, IsHardwareDevice, HasMaxVideoMemory, d3dDevice)) {
        LOG_ERROR(L"No suitable D3D12 device found.\n");
        return false;
    }

    // Wrappers for core D3D objects are below
    constexpr D3D12_COMMAND_LIST_TYPE commandListType = D3D12_COMMAND_LIST_TYPE_DIRECT;

    std::unique_ptr<CommandQueue> commandQueue;
    if (!CreateCommandQueue(d3dDevice, commandListType, D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
                            D3D12_COMMAND_QUEUE_FLAG_NONE, D3D12_FENCE_FLAG_NONE, commandQueue)) {
        LOG_ERROR(L"Failed to create the Command Queue.\n");
        return false;
    }

    std::unique_ptr<CommandAllocator> commandAllocator;
    if (!CreateCommandAllocator(d3dDevice, commandListType, D3D12_COMMAND_LIST_FLAG_NONE,
                                commandAllocator)) {
        LOG_ERROR(L"Failed to create the Command Allocator.\n");
        return false;
    }

    std::unique_ptr<DescriptorHeap> rtvHeap;
    if (!CreateDescriptorHeap(d3dDevice, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, RTV_DESCRIPTOR_COUNT,
                              rtvHeap)) {
        LOG_ERROR(L"Failed to create the RTV Descriptor Heap.\n");
        return false;
    }

    OutDevice = std::make_unique<Device>(std::move(debugLayer), std::move(rtvHeap),
                                         std::move(commandQueue), std::move(commandAllocator),
                                         std::move(dxgiFactory), std::move(d3dDevice));
    return true;
}

bool Device::GetBestDevice(ComPtr<IDXGIFactory7>& DXGIFactory,
                           D3D_FEATURE_LEVEL FeatureLevel,
                           bool IsHardwareDevice,
                           bool HasMaxVideoMemory,
                           ComPtr<ID3D12Device14>& OutD3DDevice) {
    ComPtr<IDXGIAdapter1> adapter;
    ComPtr<ID3D12Device14> bestD3dDevice;

    SIZE_T maxVideoMemory{0};
    for (uint32_t i{0}; DXGIFactory->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND; ++i) {
        // Get adapter description
        DXGI_ADAPTER_DESC1 desc;
        if (FAILED(adapter->GetDesc1(&desc))) {
            // Skip adapters that don't support GetDesc1 (e.g., non-graphics adapters)
            adapter.Reset();
            continue;
        }

        // Check is a hardware adapter
        if (IsHardwareDevice && (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)) {
            // free the ComPtr
            adapter.Reset();
            continue;
        }

        // Check adapter can create a D3D12 device
        ComPtr<ID3D12Device14> d3d12Device;
        if (FAILED(D3D12CreateDevice(adapter.Get(), FeatureLevel, IID_PPV_ARGS(&d3d12Device)))) {
            // free the ComPtr
            adapter.Reset();
            continue;
        }

        // Check adapter has the *most* dedicated video memory
        if (HasMaxVideoMemory) {
            if (desc.DedicatedVideoMemory > maxVideoMemory) {
                maxVideoMemory = desc.DedicatedVideoMemory;
                bestD3dDevice = std::move(d3d12Device);
            }
        } else {
            bestD3dDevice = std::move(d3d12Device);
        }
    }

    // Successful outcome
    if (bestD3dDevice) {
        OutD3DDevice = std::move(bestD3dDevice);
        return true;
    }

    return false;
}

bool Device::CreateCommandAllocator(ComPtr<ID3D12Device14>& D3DDevice,
                                    D3D12_COMMAND_LIST_TYPE Type,
                                    D3D12_COMMAND_LIST_FLAGS Flags,
                                    std::unique_ptr<CommandAllocator>& OutAllocator) {
    ComPtr<ID3D12CommandAllocator> commandAllocator;
    if (FAILED(D3DDevice->CreateCommandAllocator(
            // Command list type (D3D12_COMMAND_LIST_TYPE_DIRECT or D3D12_COMMAND_LIST_TYPE_COMPUTE,
            // etc)
            Type,
            // Return value
            IID_PPV_ARGS(&commandAllocator)))) {
        LOG_ERROR(L"Failed to create D3D12 command allocator.\n");
        return false;
    }

    ComPtr<ID3D12GraphicsCommandList10> graphicsCommandList;
    if (FAILED(D3DDevice->CreateCommandList1(
            // Current GPU
            0,
            // Command list type (D3D12_COMMAND_LIST_TYPE_DIRECT /
            // D3D12_COMMAND_LIST_TYPE_COMPUTE, etc)
            Type,
            // Command list flags (most likely D3D12_COMMAND_LIST_FLAG_NONE)
            Flags,
            // Return value
            IID_PPV_ARGS(&graphicsCommandList)))) {
        LOG_ERROR(L"Failed to create D3D12 command list.\n");
        return false;
    }

    OutAllocator = std::make_unique<CommandAllocator>(Type, std::move(commandAllocator),
                                                      std::move(graphicsCommandList));
    return true;
}

bool Device::CreateCommandQueue(ComPtr<ID3D12Device14>& D3DDevice,
                                D3D12_COMMAND_LIST_TYPE Type,
                                D3D12_COMMAND_QUEUE_PRIORITY Priority,
                                D3D12_COMMAND_QUEUE_FLAGS QueueFlags,
                                D3D12_FENCE_FLAGS FenceFlags,
                                std::unique_ptr<CommandQueue>& OutQueue) {
    D3D12_COMMAND_QUEUE_DESC desc{};
    desc.NodeMask = 0;  // pick the default gpu
    desc.Type = Type;
    desc.Priority = Priority;
    desc.Flags = QueueFlags;

    ComPtr<ID3D12CommandQueue> d3d12CommandQueue;
    if (FAILED(D3DDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&d3d12CommandQueue)))) {
        LOG_ERROR(L"Failed to create D3D12 command queue.\n");
        return false;
    }

    // initial fence value
    uint64_t initFenceValue = 0;

    // fence object
    ComPtr<ID3D12Fence1> d3d12Fence;
    if (FAILED(D3DDevice->CreateFence(initFenceValue, FenceFlags, IID_PPV_ARGS(&d3d12Fence)))) {
        LOG_ERROR(L"Failed to create ID3D12Fence1.\n");
        return false;
    }

    // Create an event handle for queue synchronization
    HANDLE eventHandle = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (eventHandle == nullptr) {
        LOG_ERROR(L"Failed to create event handle.\n");
        return false;
    }

    OutQueue = std::make_unique<CommandQueue>(Type, initFenceValue, eventHandle,
                                              std::move(d3d12Fence), std::move(d3d12CommandQueue));

    return true;
}

bool Device::CreateDescriptorHeap(ComPtr<ID3D12Device14>& D3DDevice,
                                  D3D12_DESCRIPTOR_HEAP_TYPE Type,
                                  uint32_t Count,
                                  std::unique_ptr<DescriptorHeap>& OutHeap) {
    D3D12_DESCRIPTOR_HEAP_DESC desc{};
    desc.Type = Type;
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    desc.NodeMask = 0;
    desc.NumDescriptors = Count;

    ComPtr<ID3D12DescriptorHeap> descriptorHeap;
    if (FAILED(D3DDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descriptorHeap)))) {
        LOG_ERROR(L"Failed to create D3D12 descriptor heap.\n");
        return false;
    }

    uint32_t descriptorSize = D3DDevice->GetDescriptorHandleIncrementSize(Type);

    OutHeap =
        std::make_unique<DescriptorHeap>(Type, descriptorSize, Count, std::move(descriptorHeap));
    return true;
}

// Instance members

void Device::CreateRenderTargetView(ID3D12Resource2* Resource,
                                    D3D12_RENDER_TARGET_VIEW_DESC& Desc,
                                    D3D12_CPU_DESCRIPTOR_HANDLE& OutCpuHandle) const {
    // Allocate a descriptor from the heap
    mRTVHeap->AllocateHandles(1, OutCpuHandle);
    // Create the RTV
    mD3DDevice->CreateRenderTargetView(Resource, &Desc, OutCpuHandle);
}

bool Device::CreateMesh(uint32_t VertexCount,
                        uint32_t VertexStrideInBytes,
                        const void* VertexData,
                        std::unique_ptr<Mesh>& OutMesh) {
    size_t dataSizeInBytes = VertexCount * VertexStrideInBytes;

    // Create a temporary upload buffer in state D3D12_RESOURCE_STATE_GENERIC_READ
    std::unique_ptr<UploadBuffer> meshGeometryUploadBuffer;
    if (!CreateBuffer(L"MeshGeometryUploadBuffer", D3D12_HEAP_TYPE_UPLOAD,
                      D3D12_RESOURCE_STATE_GENERIC_READ, dataSizeInBytes,
                      meshGeometryUploadBuffer)) {
        LOG_ERROR(L"Failed to create geometry upload buffer.\n");
        return false;
    }

    // Upload bytes to the CPU buffer
    if (!meshGeometryUploadBuffer->UploadBytes(dataSizeInBytes, VertexData)) {
        LOG_ERROR(L"Failed to upload bytes to the geometry upload buffer.\n");
        return false;
    }

    // Create the GPU vertex buffer in the state COMMON
    std::unique_ptr<DeviceBuffer> meshVertexBuffer;
    if (!CreateBuffer(L"MeshVertexBuffer", D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_COMMON,
                      dataSizeInBytes, meshVertexBuffer)) {
        LOG_ERROR(L"Failed to create vertex buffer.\n");
        return false;
    }

    // Get a command list
    CommandList10 cmdl;
    if (!this->GetCommandList(cmdl)) {
        LOG_ERROR(L"Failed to get command list.\n");
        return false;
    }

    // Transition device buffer to COPY_DEST for receiving the copy
    cmdl.TransitionResource(*meshVertexBuffer, D3D12_RESOURCE_STATE_COPY_DEST);

    // Copy from upload buffer to device buffer
    cmdl.CopyBufferRegion(*meshGeometryUploadBuffer, 0, *meshVertexBuffer,
                          meshGeometryUploadBuffer->GetBufferSize());

    // Transition device buffer to GENERIC_READ for shader access
    cmdl.TransitionResource(*meshVertexBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);

    OutMesh =
        std::make_unique<Mesh>(VertexCount, VertexStrideInBytes, std::move(*meshVertexBuffer));

    // Cmdl gets executed when exiting the scope
    return true;
}

bool Device::CreateMeshInstance(Mesh& Model, std::unique_ptr<MeshInstance>& Mesh) {
    // Each MeshInstance represents a couple of constant buffers holding transformation data about
    // the mesh
    size_t transformDataSize = sizeof(Matrix4);

    // Create an upload buffer in state D3D12_RESOURCE_STATE_GENERIC_READ (we'll never transition
    // from that)
    std::unique_ptr<UploadBuffer> uploadConstantBuffer;
    if (!CreateBuffer(L"MeshConstUploadBuffer", D3D12_HEAP_TYPE_UPLOAD,
                      D3D12_RESOURCE_STATE_GENERIC_READ, transformDataSize, uploadConstantBuffer)) {
        LOG_ERROR(L"Failed to create upload buffer.\n");
        return false;
    }

    // The buffer that holds Model's matrices
    std::unique_ptr<DeviceBuffer> meshConstantBuffer;
    if (!CreateBuffer(L"MeshConstDeviceBuffer", D3D12_HEAP_TYPE_DEFAULT,
                      D3D12_RESOURCE_STATE_COMMON, transformDataSize, meshConstantBuffer)) {
        LOG_ERROR(L"Failed to create default buffer.\n");
        return false;
    }

    Mesh = std::make_unique<MeshInstance>(Model, std::move(uploadConstantBuffer),
                                          std::move(meshConstantBuffer));
    return true;
}

bool Device::CreateMeshNode(MaterialId MaterialId, Mesh& Model, std::unique_ptr<Node>& OutNode) {
    // Create a mesh instance
    std::unique_ptr<MeshInstance> MeshInstance;
    if (!CreateMeshInstance(Model, MeshInstance)) {
        LOG_ERROR(L"Failed to create mesh instance.\n");
        return false;
    }

    // Combine both the mesh and the materialId into a scene node
    OutNode = std::make_unique<Node>(MaterialId, std::move(MeshInstance));
    return true;
}

bool Device::CreateRootSignature(ByteBuffer& Bytecode,
                                 std::unique_ptr<RootSignature>& OutRootSignature) const {
    ComPtr<ID3D12RootSignature> rootSignature;
    if (FAILED(mD3DDevice->CreateRootSignature(0, Bytecode.GetBuffer(), Bytecode.GetSize(),
                                               IID_PPV_ARGS(&rootSignature)))) {
        LOG_ERROR(L"Failed to create root signature.\n");
        return false;
    }

    OutRootSignature = std::make_unique<RootSignature>(std::move(rootSignature));
    return true;
}

bool Device::CreatePipelineState(D3D12_GRAPHICS_PIPELINE_STATE_DESC& PsoDesc,
                                 std::unique_ptr<PipelineState>& OutPSO) const {
    ComPtr<ID3D12PipelineState> pipelineState;
    if (FAILED(mD3DDevice->CreateGraphicsPipelineState(&PsoDesc, IID_PPV_ARGS(&pipelineState)))) {
        LOG_ERROR(L"Failed to create pipeline state.\n");
        return false;
    }

    OutPSO = std::make_unique<PipelineState>(std::move(pipelineState));
    return true;
}

bool Device::CreateSwapChain(HWND HWnd,
                             uint32_t Width,
                             uint32_t Height,
                             std::unique_ptr<SwapChain>& OutSwapChain) {
    uint32_t bufferCount = SWAP_CHAIN_BUFFER_COUNT;

    // DXGI_FORMAT_R8G8B8A8_UNORM is the most common swap chain format
    DXGI_FORMAT bufferFormat = DXGI_FORMAT_DEFAULT_RTV;

    DXGI_USAGE bufferUsage =
        // Back buffer (target for rendering)
        DXGI_USAGE_BACK_BUFFER |
        // output to a window (hwnd)
        DXGI_USAGE_RENDER_TARGET_OUTPUT;

    uint32_t bufferFlags =
        // Create a flip-model swap chain
        DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH |
        // Enable tearing capability (actual behavior controlled by Present() call)
        DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

    // Swap chain desc
    DXGI_SWAP_CHAIN_DESC1 desc{};
    desc.Width = Width;
    desc.Height = Height;
    desc.Format = bufferFormat;
    desc.Flags = bufferFlags;

    // No stereo
    desc.Stereo = FALSE;

    // no multi-sampling, pixel per pixel
    desc.SampleDesc.Count = 1;
    // no anti-aliasing
    desc.SampleDesc.Quality = 0;

    desc.BufferCount = bufferCount;
    desc.BufferUsage = bufferUsage;
    // Stretch the image until the SwapChain::Resize is called
    desc.Scaling = DXGI_SCALING_STRETCH;

    // Flip discard
    desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

    // Always ignore
    desc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

    // Fullscreen desc
    DXGI_SWAP_CHAIN_FULLSCREEN_DESC descFullscreen{};
    descFullscreen.Windowed = true;

    // Create the swap chain as IDXGISwapChain1 first
    ComPtr<IDXGISwapChain1> swapChain1;
    if (FAILED(mDXGIFactory->CreateSwapChainForHwnd(mCommandQueue->GetD3D12CommandQueue(), HWnd,
                                                    &desc, &descFullscreen, nullptr,
                                                    &swapChain1))) {
        LOG_ERROR(L"Failed to create DXGI swap chain.\n");
        return false;
    }

    // Query for IDXGISwapChain4 as we need it for GetCurrentBackBufferIndex()
    ComPtr<IDXGISwapChain4> dxgiSwapChain;
    if (FAILED(swapChain1.As(&dxgiSwapChain))) {
        LOG_ERROR(L"Failed to query IDXGISwapChain4 interface.\n");
        return false;
    }

    if (bufferCount < 2) {
        LOG_ERROR(L"Swap chain buffer count must be at least 2.\n");
        return false;
    }

    OutSwapChain = std::make_unique<SwapChain>(bufferCount, bufferFormat, bufferFlags, *this,
                                               *mCommandQueue, std::move(dxgiSwapChain));
    return true;
}

bool Device::GetCommandList(CommandList10& OutCommandList) const {
    ID3D12GraphicsCommandList10* d3dCommandList;
    if (!mCommandAllocator->GetID3D12CommandList(d3dCommandList)) {
        LOG_ERROR(L"Failed to get command list from the allocator.\n");
        return false;
    }

    OutCommandList = CommandList10(mCommandQueue.get(), d3dCommandList);
    return true;
}

bool Device::GetFrameCommandList(SwapChain& SwapChain, FrameCommandList10& OutCommandList) const {
    ID3D12GraphicsCommandList10* d3dCommandList;
    if (!mCommandAllocator->GetID3D12CommandList(d3dCommandList)) {
        LOG_ERROR(L"Failed to get command list from the allocator.\n");
        return false;
    }

    OutCommandList = FrameCommandList10(&SwapChain, mCommandQueue.get(), d3dCommandList);
    return true;
}
