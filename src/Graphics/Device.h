#pragma once

#include <memory>

#include "CommandAllocator.h"
#include "CommandQueue.h"
#include "DebugLayer.h"
#include "DescriptorHeap.h"
#include "IO/ByteBuffer.h"
#include "Includes/ComIncl.h"
#include "Includes/GraphicsIncl.h"
#include "Logging/Logging.h"
#include "Material/Material.h"
#include "Material/PipelineState.h"
#include "Material/RootSignature.h"
#include "Mesh/Mesh.h"
#include "Mesh/MeshInstance.h"
#include "Resource/DeviceBuffer.h"
#include "Scene/Node.h"
#include "SwapChain.h"

// Graphics configs below
constexpr D3D_FEATURE_LEVEL GRAPHICS_FEATURE_LEVEL = D3D_FEATURE_LEVEL_12_0;

// number of RTV descriptors to allocate in the heap
constexpr uint32_t RTV_DESCRIPTOR_COUNT{256};

// count is 2 to accommodate back buffer (one is presenting while the other is a back buffer)
constexpr uint32_t SWAP_CHAIN_BUFFER_COUNT{2};

// Forward declarations
class ByteBuffer;
class CommandList10;
class FrameCommandList10;

/**
 * Device class encapsulates the D3D12 device and related resources.
 */
class Device {
    // Alias for Microsoft::WRL::ComPtr
    template <typename T>
    using ComPtr = Microsoft::WRL::ComPtr<T>;

   public:
    /**
     * Creates Device instance using default GRAPHICS_FEATURE_LEVEL (D3D_FEATURE_LEVEL_12_0) and
     * looking for the hardware device with the highest amount of video memory.
     *
     * @param OutDevice Output parameter that will be populated with the created Device instance on
     * success. Unchanged on failure.
     * @return true if the Device was successfully created, false otherwise (e.g., no suitable
     * device found or initialization failed).
     */
    static bool Create(std::unique_ptr<Device>& OutDevice) {
        return Create(GRAPHICS_FEATURE_LEVEL, true, true, OutDevice);
    }

    /**
     * Creates Device instance with specified feature level and device selection criteria.
     *
     * @param FeatureLevel The minimum DirectX feature level required (e.g.,
     * D3D_FEATURE_LEVEL_12_0).
     * @param IsHardwareDevice If true, only hardware adapters are considered. If false, software
     * adapters may be used.
     * @param HasMaxVideoMemory If true, selects the adapter with the highest dedicated video
     * memory. If false, uses the first suitable adapter found.
     * @param OutDevice Output parameter that will be populated with the created Device instance on
     * success. Unchanged on failure.
     * @return true if the Device was successfully created, false otherwise (e.g., no suitable
     * device found or initialization failed).
     */
    static bool Create(D3D_FEATURE_LEVEL FeatureLevel,
                       bool IsHardwareDevice,
                       bool HasMaxVideoMemory,
                       std::unique_ptr<Device>& OutDevice);

    /**
     * Creates a command allocator and associated command list for the specified command list type.
     *
     * @param mD3DDevice The D3D12 device to create the allocator with.
     * @param Type The type of command list (e.g., D3D12_COMMAND_LIST_TYPE_DIRECT,
     * D3D12_COMMAND_LIST_TYPE_COMPUTE).
     * @param Flags Flags for the command list creation (typically D3D12_COMMAND_LIST_FLAG_NONE).
     * @param OutAllocator Output parameter that will be populated with the created CommandAllocator
     * instance on success. Unchanged on failure.
     * @return true if the CommandAllocator was successfully created, false otherwise.
     */
    static bool CreateCommandAllocator(ComPtr<ID3D12Device14>& D3DDevice,
                                       D3D12_COMMAND_LIST_TYPE Type,
                                       D3D12_COMMAND_LIST_FLAGS Flags,
                                       std::unique_ptr<CommandAllocator>& OutAllocator);

    /**
     * Creates a command queue with the specified configuration, including a fence for
     * synchronization.
     *
     * @param mD3DDevice The D3D12 device to create the queue with.
     * @param Type The type of command queue (e.g., D3D12_COMMAND_LIST_TYPE_DIRECT).
     * @param Priority The priority of the command queue (e.g.,
     * D3D12_COMMAND_QUEUE_PRIORITY_NORMAL).
     * @param QueueFlags Flags for the command queue (typically D3D12_COMMAND_QUEUE_FLAG_NONE).
     * @param FenceFlags Flags for the fence creation (typically D3D12_FENCE_FLAG_NONE).
     * @param OutQueue Output parameter that will be populated with the created CommandQueue
     * instance on success. Unchanged on failure.
     * @return true if the CommandQueue was successfully created, false otherwise.
     */
    static bool CreateCommandQueue(ComPtr<ID3D12Device14>& D3DDevice,
                                   D3D12_COMMAND_LIST_TYPE Type,
                                   D3D12_COMMAND_QUEUE_PRIORITY Priority,
                                   D3D12_COMMAND_QUEUE_FLAGS QueueFlags,
                                   D3D12_FENCE_FLAGS FenceFlags,
                                   std::unique_ptr<CommandQueue>& OutQueue);

    /**
     * Creates a descriptor heap of the specified type and size.
     *
     * @param mD3DDevice The D3D12 device to create the heap with.
     * @param Type The type of descriptor heap (e.g., D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
     * D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV).
     * @param Count The number of descriptors to allocate in the heap.
     * @param OutHeap Output parameter that will be populated with the created DescriptorHeap
     * instance on success. Unchanged on failure.
     * @return true if the DescriptorHeap was successfully created, false otherwise.
     */
    static bool CreateDescriptorHeap(ComPtr<ID3D12Device14>& D3DDevice,
                                     D3D12_DESCRIPTOR_HEAP_TYPE Type,
                                     uint32_t Count,
                                     std::unique_ptr<DescriptorHeap>& OutHeap);

    /**
     * Enumerates available adapters and selects the best D3D12 device based on the specified
     * criteria.
     *
     * @param DXGIFactory The DXGI factory used to enumerate adapters.
     * @param FeatureLevel The minimum DirectX feature level required.
     * @param IsHardwareDevice If true, only hardware adapters are considered. Software adapters are
     * skipped.
     * @param HasMaxVideoMemory If true, selects the adapter with the highest dedicated video
     * memory. If false, returns the first suitable adapter found.
     * @param OutD3DDevice Output parameter that will be populated with the selected D3D12 device on
     * success. Unchanged on failure.
     * @return true if a suitable device was found and created, false otherwise.
     */
    static bool GetBestDevice(ComPtr<IDXGIFactory7>& DXGIFactory,
                              D3D_FEATURE_LEVEL FeatureLevel,
                              bool IsHardwareDevice,
                              bool HasMaxVideoMemory,
                              ComPtr<ID3D12Device14>& OutD3DDevice);

    Device(std::unique_ptr<DebugLayer>&& DebugLayer,
           std::unique_ptr<DescriptorHeap> RtvHeap,
           std::unique_ptr<CommandQueue>&& CommandQueue,
           std::unique_ptr<CommandAllocator>&& CommandAllocator,
           ComPtr<IDXGIFactory7>&& DXGIFactory,
           ComPtr<ID3D12Device14> D3DDevice)
        : mDebugLayer{std::move(DebugLayer)},

          mCommandQueue{std::move(CommandQueue)},
          mCommandAllocator{std::move(CommandAllocator)},

          mRTVHeap{std::move(RtvHeap)},
          mDXGIFactory{std::move(DXGIFactory)},
          mD3DDevice{std::move(D3DDevice)} {}

    ~Device() {
        LOG_INFO(L"Freeing Device.\n");
    }

    // Deleted copy constructor and assignment operator to prevent copying
    Device(Device& Copy) = delete;
    Device& operator=(const Device& Copy) = delete;

    // Instance members
    /**
     * Creates a graphics pipeline state object from the provided description.
     *
     * @param Desc The graphics pipeline state description containing shaders, render targets, blend
     * states, etc.
     * @param OutPSO Output parameter that will be populated with the created PipelineState instance
     * on success. Unchanged on failure.
     * @return true if the PipelineState was successfully created, false otherwise.
     */
    bool CreatePipelineState(D3D12_GRAPHICS_PIPELINE_STATE_DESC& PsoDesc,
                             std::unique_ptr<PipelineState>& OutPSO) const;

    /**
     * Creates a render target view (RTV) for the specified resource and allocates a descriptor from
     * the RTV heap.
     *
     * @param pResource The D3D12 resource to create the RTV for.
     * @param desc The render target view description.
     * @param OutCpuHandle Output parameter that will be populated with the CPU descriptor handle
     * for the created RTV.
     */
    void CreateRenderTargetView(ID3D12Resource2* Resource,
                                D3D12_RENDER_TARGET_VIEW_DESC& Desc,
                                D3D12_CPU_DESCRIPTOR_HANDLE& OutCpuHandle) const;

    /**
     * Creates a root signature from compiled shader bytecode.
     *
     * @param Bytecode The Bytes object containing the compiled root signature bytecode.
     * @param OutRootSignature Output parameter that will be populated with the created
     * RootSignature instance on success. Unchanged on failure.
     * @return true if the RootSignature was successfully created, false otherwise.
     */
    bool CreateRootSignature(ByteBuffer& Bytecode,
                             std::unique_ptr<RootSignature>& OutRootSignature) const;

    /**
     * Creates a swap chain for the specified window with the given dimensions.
     * The swap chain uses flip-model presentation with tearing support enabled.
     *
     * @param hWnd The window handle to associate with the swap chain.
     * @param Width The width of the swap chain buffers in pixels.
     * @param Height The height of the swap chain buffers in pixels.
     * @param OutSwapChain Output parameter that will be populated with the created SwapChain
     * instance on success. Unchanged on failure.
     * @return true if the SwapChain was successfully created, false otherwise.
     */
    bool CreateSwapChain(HWND HWnd,
                         uint32_t Width,
                         uint32_t Height,
                         std::unique_ptr<SwapChain>& OutSwapChain);

    /**
     * Creates a buffer resource of the specified type and size. The buffer size is automatically
     * aligned to 256 bytes as required by D3D12.
     *
     * @tparam T The buffer type, must be derived from DeviceBuffer (e.g., UploadBuffer,
     * DeviceBuffer).
     * @param Name The debug name for the buffer resource.
     * @param Type The heap type for the buffer (e.g., D3D12_HEAP_TYPE_UPLOAD,
     * D3D12_HEAP_TYPE_DEFAULT).
     * @param Size The size of the buffer in bytes (will be aligned to 256 bytes).
     * @param OutBuffer Output parameter that will be populated with the created buffer instance on
     * success. Unchanged on failure.
     * @return true if the buffer was successfully created, false otherwise.
     */
    template <typename T>
    bool CreateBuffer(std::wstring BufferName,
                      D3D12_HEAP_TYPE Type,
                      D3D12_RESOURCE_STATES State,
                      size_t Size,
                      std::unique_ptr<T>& OutBuffer)
        requires std::is_base_of_v<DeviceBuffer, T>
    {
        size_t BufferSize = ByteBuffer::AlignTo256Bytes(Size);

        ComPtr<ID3D12Resource2> d3dBuffer;
        CD3DX12_HEAP_PROPERTIES heapProps{Type};
        CD3DX12_RESOURCE_DESC bufferDesc{CD3DX12_RESOURCE_DESC::Buffer(BufferSize)};
        if (FAILED(mD3DDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE,
                                                       &bufferDesc, State, nullptr,
                                                       IID_PPV_ARGS(&d3dBuffer)))) {
            LOG_ERROR(L"Failed to create buffer.\n");
            return false;
        }
        d3dBuffer->SetName(BufferName.c_str());
        OutBuffer = std::make_unique<T>(Type, BufferSize, d3dBuffer);
        return true;
    }

    /**
     * Creates a mesh from vertex data. The vertex data is uploaded to the GPU via an upload buffer
     * and copied to a default heap vertex buffer.
     *
     * @param VertexCount The number of vertices in the mesh.
     * @param VertexStrideInBytes The size of a single vertex in bytes.
     * @param Data Pointer to the vertex data to upload.
     * @param OutMesh Output parameter that will be populated with the created Mesh instance on
     * success. Unchanged on failure.
     * @return true if the Mesh was successfully created, false otherwise.
     */
    bool CreateMesh(uint32_t VertexCount,
                    uint32_t VertexStrideInBytes,
                    const void* VertexData,
                    std::unique_ptr<Mesh>& OutMesh);

    /**
     * Creates a mesh instance that combines a mesh with a material for rendering. The instance
     * includes CPU and GPU buffers for per-instance constant data.
     *
     * @param Model The shared mesh to use for this instance.
     * @param Mesh Output parameter that will be populated with the created MeshInstance
     * on success. Unchanged on failure.
     * @return true if the MeshInstance was successfully created, false otherwise.
     */
    bool CreateMeshInstance(Mesh& Model, std::unique_ptr<MeshInstance>& Mesh);

    bool CreateMeshNode(MaterialId MaterialId, Mesh& Model, std::unique_ptr<Node>& OutNode);

    /**
     * Retrieves a command list from the device's command allocator for recording graphics commands.
     *
     * @param OutCommandList Output parameter that will be populated with the CommandList10 instance
     * on success. Unchanged on failure.
     * @return true if the command list was successfully retrieved, false otherwise.
     */
    bool GetCommandList(CommandList10& OutCommandList) const;

    /**
     * Retrieves a frame command list associated with a swap chain for recording per-frame rendering
     * commands.
     *
     * @param SwapChain The swap chain to associate with the command list.
     * @param OutCommandList Output parameter that will be populated with the FrameCommandList10
     * instance on success. Unchanged on failure.
     * @return true if the frame command list was successfully retrieved, false otherwise.
     */
    bool GetFrameCommandList(SwapChain& SwapChain, FrameCommandList10& OutCommandList) const;

    /**
     * Disables the Alt+Enter keyboard shortcut that toggles fullscreen mode for the specified
     * window.
     *
     * @param mHWnd The window handle to disable the fullscreen toggle for.
     */
    void DisableAltEnterFullscreenToggle(HWND HWnd) const {
        // Disable the Alt+Enter fullscreen toggle feature. Switching to fullscreen
        mDXGIFactory->MakeWindowAssociation(HWnd, DXGI_MWA_NO_ALT_ENTER);
    }

   private:
    // IMPORTANT! Keep the DebugLayer at the very top to ensure it is destroyed the last.
    // It reports on LIVE DX objects before the context is destroyed.
    std::unique_ptr<DebugLayer> mDebugLayer;

    std::unique_ptr<DescriptorHeap> mRTVHeap;
    std::unique_ptr<CommandQueue> mCommandQueue;
    std::unique_ptr<CommandAllocator> mCommandAllocator;

    ComPtr<IDXGIFactory7> mDXGIFactory;
    ComPtr<ID3D12Device14> mD3DDevice;
};
