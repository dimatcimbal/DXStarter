#include "Device.h"

#include <vector>

#include "CommandList10.h"
#include "IO/Bytes.h"
#include "Logging/Logging.h"
#include "Material/RootSignature.h"
#include "Mesh/MeshInstance.h"
#include "Resource/UploadBuffer.h"

using Microsoft::WRL::ComPtr;

bool Device::Create(D3D_FEATURE_LEVEL FeatureLevel,
                    bool IsHardwareDevice,
                    bool HasMaxVideoMemory,
                    std::unique_ptr<Device>& OutDevice) {
    std::unique_ptr<DebugLayer> pDebugLayer;
    if (!DebugLayer::Create(pDebugLayer)) {
        LOG_ERROR(L"\tFailed to initialize the Debug Layer.\n");
        return false;
    }

    ComPtr<IDXGIFactory7> pDXGIFactory;
    if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&pDXGIFactory)))) {
        LOG_ERROR(L"\t\tFailed to create DXGI factory.\n");
        return false;
    }

    ComPtr<ID3D12Device14> pD3DDevice;
    if (!GetBestDevice(pDXGIFactory, FeatureLevel, IsHardwareDevice, HasMaxVideoMemory,
                       pD3DDevice)) {
        LOG_ERROR(L"\t\tNo suitable D3D12 device found.\n");
        return false;
    }

    // Wrappers for core D3D objects are below
    constexpr D3D12_COMMAND_LIST_TYPE CommandListType = D3D12_COMMAND_LIST_TYPE_DIRECT;

    std::unique_ptr<CommandQueue> pCommandQueue;
    if (!CreateCommandQueue(pD3DDevice, CommandListType, D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
                            D3D12_COMMAND_QUEUE_FLAG_NONE, D3D12_FENCE_FLAG_NONE, pCommandQueue)) {
        LOG_ERROR(L"\tFailed to create the Command Queue.\n");
        return false;
    }

    std::unique_ptr<CommandAllocator> pCommandAllocator;
    if (!CreateCommandAllocator(pD3DDevice, CommandListType, D3D12_COMMAND_LIST_FLAG_NONE,
                                pCommandAllocator)) {
        LOG_ERROR(L"\tFailed to create the Command Allocator.\n");
        return false;
    }

    std::unique_ptr<DescriptorHeap> pRTVHeap;
    if (!CreateDescriptorHeap(pD3DDevice, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, RTV_DESCRIPTOR_COUNT,
                              pRTVHeap)) {
        LOG_ERROR(L"\tFailed to create the RTV Descriptor Heap.\n");
        return false;
    }

    OutDevice = std::make_unique<Device>(std::move(pDebugLayer), std::move(pRTVHeap),
                                         std::move(pCommandQueue), std::move(pCommandAllocator),
                                         std::move(pDXGIFactory), std::move(pD3DDevice));
    return true;
}

bool Device::GetBestDevice(ComPtr<IDXGIFactory7>& DXGIFactory,
                           D3D_FEATURE_LEVEL FeatureLevel,
                           bool IsHardwareDevice,
                           bool HasMaxVideoMemory,
                           ComPtr<ID3D12Device14>& OutD3DDevice) {
    ComPtr<IDXGIAdapter1> pAdapter;
    ComPtr<ID3D12Device14> pBestD3DDevice;

    SIZE_T maxVideoMemory{0};
    for (uint32_t i{0}; DXGIFactory->EnumAdapters1(i, &pAdapter) != DXGI_ERROR_NOT_FOUND; ++i) {
        // Get adapter description
        DXGI_ADAPTER_DESC1 Desc;
        if (FAILED(pAdapter->GetDesc1(&Desc))) {
            // Skip adapters that don't support GetDesc1 (e.g., non-graphics adapters)
            pAdapter.Reset();
            continue;
        }

        // Check is a hardware adapter
        if (IsHardwareDevice && (Desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)) {
            // free the ComPtr
            pAdapter.Reset();
            continue;
        }

        // Check adapter can create a D3D12 device
        ComPtr<ID3D12Device14> pD3D12Device;
        if (FAILED(D3D12CreateDevice(pAdapter.Get(), FeatureLevel, IID_PPV_ARGS(&pD3D12Device)))) {
            // free the ComPtr
            pAdapter.Reset();
            continue;
        }

        // Check adapter has the *most* dedicated video memory
        if (HasMaxVideoMemory) {
            if (Desc.DedicatedVideoMemory > maxVideoMemory) {
                maxVideoMemory = Desc.DedicatedVideoMemory;
                pBestD3DDevice = std::move(pD3D12Device);
            }
        } else {
            pBestD3DDevice = std::move(pD3D12Device);
        }
    }

    // Successful outcome
    if (pBestD3DDevice) {
        OutD3DDevice = std::move(pBestD3DDevice);
        return true;
    }

    return false;
}

bool Device::CreateCommandAllocator(ComPtr<ID3D12Device14>& mD3DDevice,
                                    D3D12_COMMAND_LIST_TYPE Type,
                                    D3D12_COMMAND_LIST_FLAGS Flags,
                                    std::unique_ptr<CommandAllocator>& OutAllocator) {
    ComPtr<ID3D12CommandAllocator> pCommandAllocator;
    if (FAILED(mD3DDevice->CreateCommandAllocator(
            // Command list type (D3D12_COMMAND_LIST_TYPE_DIRECT or D3D12_COMMAND_LIST_TYPE_COMPUTE,
            // etc)
            Type,
            // Return value
            IID_PPV_ARGS(&pCommandAllocator)))) {
        LOG_ERROR(L"\t\tFailed to create D3D12 command allocator.\n");
        return false;
    }

    ComPtr<ID3D12GraphicsCommandList10> pGraphicsCommandList;
    if (FAILED(mD3DDevice->CreateCommandList1(
            // Current GPU
            0,
            // Command list type (D3D12_COMMAND_LIST_TYPE_DIRECT /
            // D3D12_COMMAND_LIST_TYPE_COMPUTE, etc)
            Type,
            // Command list flags (most likely D3D12_COMMAND_LIST_FLAG_NONE)
            Flags,
            // Return value
            IID_PPV_ARGS(&pGraphicsCommandList)))) {
        LOG_ERROR(L"\t\tFailed to create D3D12 command list.\n");
        return false;
    }

    OutAllocator = std::make_unique<CommandAllocator>(Type, std::move(pCommandAllocator),
                                                      std::move(pGraphicsCommandList));
    return true;
}

bool Device::CreateCommandQueue(ComPtr<ID3D12Device14>& mD3DDevice,
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

    ComPtr<ID3D12CommandQueue> pD3D12CommandQueue;
    if (FAILED(mD3DDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&pD3D12CommandQueue)))) {
        LOG_ERROR(L"\t\tFailed to create D3D12 command queue.\n");
        return false;
    }

    // initial fence value
    uint64_t InitFenceValue = 0;

    // fence object
    ComPtr<ID3D12Fence1> pD3D12Fence;
    if (FAILED(mD3DDevice->CreateFence(InitFenceValue, FenceFlags, IID_PPV_ARGS(&pD3D12Fence)))) {
        LOG_ERROR(L"\t\tFailed to create ID3D12Fence1.\n");
        return false;
    }

    // Create an event handle for queue synchronization
    HANDLE EventHandle = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (EventHandle == nullptr) {
        LOG_ERROR(L"Failed to create event handle.\n");
        return false;
    }

    OutQueue = std::make_unique<CommandQueue>(
        Type, InitFenceValue, EventHandle, std::move(pD3D12Fence), std::move(pD3D12CommandQueue));

    return true;
}

bool Device::CreateDescriptorHeap(ComPtr<ID3D12Device14>& mD3DDevice,
                                  D3D12_DESCRIPTOR_HEAP_TYPE Type,
                                  uint32_t Count,
                                  std::unique_ptr<DescriptorHeap>& OutHeap) {
    D3D12_DESCRIPTOR_HEAP_DESC desc{};
    desc.Type = Type;
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    desc.NodeMask = 0;
    desc.NumDescriptors = Count;

    ComPtr<ID3D12DescriptorHeap> pDescriptorHeap;
    if (FAILED(mD3DDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&pDescriptorHeap)))) {
        LOG_ERROR(L"\t\tFailed to create D3D12 descriptor heap.\n");
        return false;
    }

    uint32_t DescriptorSize = mD3DDevice->GetDescriptorHandleIncrementSize(Type);

    OutHeap =
        std::make_unique<DescriptorHeap>(Type, DescriptorSize, Count, std::move(pDescriptorHeap));
    return true;
}

// Instance members

void Device::CreateRenderTargetView(ID3D12Resource2* pResource,
                                    D3D12_RENDER_TARGET_VIEW_DESC& desc,
                                    D3D12_CPU_DESCRIPTOR_HANDLE& OutHandle) const {
    // Allocate a descriptor from the heap
    mRTVHeap->AllocateHandles(1, OutHandle);
    // Create the RTV
    mD3DDevice->CreateRenderTargetView(pResource, &desc, OutHandle);
}

bool Device::CreateMesh(uint32_t VertexCount,
                        uint32_t VertexStrideInBytes,
                        const void* Data,
                        std::shared_ptr<Mesh>& OutMesh) {
    size_t DataSizeInBytes = VertexCount * VertexStrideInBytes;

    // Create temporary CPU buffer
    std::unique_ptr<UploadBuffer> MeshGeometryUploadBuffer;
    if (!CreateBuffer(L"MeshGeometryUploadBuffer", D3D12_HEAP_TYPE_UPLOAD, DataSizeInBytes,
                      MeshGeometryUploadBuffer)) {
        LOG_ERROR(L"Failed to create geometry upload buffer.\n");
        return false;
    }

    // Upload bytes to the CPU buffer
    if (!MeshGeometryUploadBuffer->UploadBytes(DataSizeInBytes, Data)) {
        LOG_ERROR(L"Failed to upload bytes to the geometry upload buffer.\n");
        return false;
    }

    // Create the GPU vertex buffer
    std::unique_ptr<ByteBuffer> MeshVertexBuffer;
    if (!CreateBuffer(L"MeshVertexBuffer", D3D12_HEAP_TYPE_DEFAULT, DataSizeInBytes,
                      MeshVertexBuffer)) {
        LOG_ERROR(L"Failed to create vertex buffer.\n");
        return false;
    }

    // Get a command list
    CommandList10 Cmdl;
    if (!this->GetCommandList(Cmdl)) {
        LOG_ERROR(L"Failed to get command list.\n");
        return false;
    }

    // Transition the upload buffer to GENERIC_READ
    Cmdl.TransitionResource(*MeshGeometryUploadBuffer,
                            // From state
                            D3D12_RESOURCE_STATE_COMMON,
                            // To state
                            D3D12_RESOURCE_STATE_GENERIC_READ);

    // Transition the vertex buffer to COPY_DEST
    Cmdl.TransitionResource(*MeshVertexBuffer,
                            // From state
                            D3D12_RESOURCE_STATE_COMMON,
                            // To state
                            D3D12_RESOURCE_STATE_COPY_DEST);

    // 5. Copy from the upload buffer to the vertex buffer
    Cmdl.CopyBufferRegion(
        // From
        *MeshGeometryUploadBuffer, 0,
        // To
        *MeshVertexBuffer, DataSizeInBytes);

    OutMesh =
        std::make_shared<Mesh>(VertexCount, VertexStrideInBytes, std::move(*MeshVertexBuffer));

    // Cmdl gets executed when exiting the scope
    return true;
}

bool Device::CreateMeshInstance(std::shared_ptr<Mesh> Mesh,
                                std::shared_ptr<Material> Material,
                                std::unique_ptr<MeshInstance>& OutModelInstance) {
    size_t VertexDataSize = Mesh->GetVertexBufferSize();

    std::unique_ptr<UploadBuffer> MeshConstCpuBuffer;
    if (!CreateBuffer(L"MeshConstUploadBuffer", D3D12_HEAP_TYPE_UPLOAD, VertexDataSize,
                      MeshConstCpuBuffer)) {
        LOG_ERROR(L"Failed to create upload buffer.\n");
        return false;
    }

    // The buffer that holds Model's matrices
    std::unique_ptr<ByteBuffer> MeshConstGpuBuffer;
    if (!CreateBuffer(L"MeshConstGpuBuffer", D3D12_HEAP_TYPE_DEFAULT, VertexDataSize,
                      MeshConstGpuBuffer)) {
        LOG_ERROR(L"Failed to create default buffer.\n");
        return false;
    }

    OutModelInstance = std::make_unique<MeshInstance>(std::move(Mesh), std::move(Material),
                                                      std::move(MeshConstCpuBuffer),
                                                      std::move(MeshConstGpuBuffer));
    return true;
}

bool Device::CreateRootSignature(Bytes& Source,
                                 std::unique_ptr<RootSignature>& OutRootSignature) const {
    ComPtr<ID3D12RootSignature> pRootSignature;
    if (FAILED(mD3DDevice->CreateRootSignature(0, Source.GetBuffer(), Source.GetSize(),
                                               IID_PPV_ARGS(&pRootSignature)))) {
        LOG_ERROR(L"Failed to create root signature.\n");
        return false;
    }

    OutRootSignature = std::make_unique<RootSignature>(std::move(pRootSignature));
    return true;
}

bool Device::CreatePipelineState(D3D12_GRAPHICS_PIPELINE_STATE_DESC& Desc,
                                 std::unique_ptr<PipelineState>& OutPSO) const {
    ComPtr<ID3D12PipelineState> pPipelineState;
    if (FAILED(mD3DDevice->CreateGraphicsPipelineState(&Desc, IID_PPV_ARGS(&pPipelineState)))) {
        LOG_ERROR(L"Failed to create pipeline state.\n");
        return false;
    }

    OutPSO = std::make_unique<PipelineState>(std::move(pPipelineState));
    return true;
}

bool Device::CreateSwapChain(HWND hWnd,
                             uint32_t Width,
                             uint32_t Height,
                             std::unique_ptr<SwapChain>& OutSwapChain) {
    uint32_t BufferCount = SWAP_CHAIN_BUFFER_COUNT;

    // DXGI_FORMAT_R8G8B8A8_UNORM is the most common swap chain format
    DXGI_FORMAT BufferFormat = DXGI_FORMAT_DEFAULT_RTV;

    DXGI_USAGE BufferUsage =
        // Back buffer (target for rendering)
        DXGI_USAGE_BACK_BUFFER |
        // output to a window (hwnd)
        DXGI_USAGE_RENDER_TARGET_OUTPUT;

    uint32_t BufferFlags =
        // Create a flip-model swap chain
        DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH |
        // Enable tearing capability (actual behavior controlled by Present() call)
        DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

    // Swap chain desc
    DXGI_SWAP_CHAIN_DESC1 desc{};
    desc.Width = Width;
    desc.Height = Height;
    desc.Format = BufferFormat;
    desc.Flags = BufferFlags;

    // No stereo
    desc.Stereo = FALSE;

    // no multi-sampling, pixel per pixel
    desc.SampleDesc.Count = 1;
    // no anti-aliasing
    desc.SampleDesc.Quality = 0;

    desc.BufferCount = BufferCount;
    desc.BufferUsage = BufferUsage;
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
    ComPtr<IDXGISwapChain1> pSwapChain1;
    if (FAILED(mDXGIFactory->CreateSwapChainForHwnd(mCommandQueue->GetD3D12CommandQueue(), hWnd,
                                                    &desc, &descFullscreen, nullptr,
                                                    &pSwapChain1))) {
        LOG_ERROR(L"\t\tFailed to create DXGI swap chain.\n");
        return false;
    }

    // Query for IDXGISwapChain4 as we need it for GetCurrentBackBufferIndex()
    ComPtr<IDXGISwapChain4> pDXGISwapChain;
    if (FAILED(pSwapChain1.As(&pDXGISwapChain))) {
        LOG_ERROR(L"\t\tFailed to query IDXGISwapChain4 interface.\n");
        return false;
    }

    if (BufferCount < 2) {
        LOG_ERROR(L"\t\tSwap chain buffer count must be at least 2.\n");
        return false;
    }

    OutSwapChain = std::make_unique<SwapChain>(BufferCount, BufferFormat, BufferFlags, *this,
                                               *mCommandQueue, std::move(pDXGISwapChain));
    return true;
}

bool Device::GetCommandList(CommandList10& OutCommandList) const {
    ID3D12GraphicsCommandList10* pD3DCommandList;
    if (!mCommandAllocator->GetID3D12CommandList(pD3DCommandList)) {
        LOG_ERROR(L"Failed to get command list from the allocator.\n");
        return false;
    }

    OutCommandList = CommandList10(mCommandQueue.get(), pD3DCommandList);
    return true;
}

bool Device::GetFrameCommandList(SwapChain& SwapChain, FrameCommandList10& OutCommandList) const {
    ID3D12GraphicsCommandList10* pD3DCommandList;
    if (!mCommandAllocator->GetID3D12CommandList(pD3DCommandList)) {
        LOG_ERROR(L"Failed to get command list from the allocator.\n");
        return false;
    }

    OutCommandList = FrameCommandList10(&SwapChain, mCommandQueue.get(), pD3DCommandList);
    return true;
}
