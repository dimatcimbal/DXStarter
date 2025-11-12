#pragma once

#include <memory>

#include "ByteUtil.h"
#include "CommandAllocator.h"
#include "CommandQueue.h"
#include "DebugLayer.h"
#include "DescriptorHeap.h"
#include "Includes/ComIncl.h"
#include "Includes/GraphicsIncl.h"
#include "Logging/Logging.h"
#include "Mesh/Mesh.h"
#include "Mesh/MeshInstance.h"
#include "PipelineState.h"
#include "Resources/ByteBuffer.h"
#include "RootSignature.h"
#include "SwapChain.h"

class Bytes;
// Forward declarations
class FrameCommandList10;

// Graphics configs below
constexpr D3D_FEATURE_LEVEL GRAPHICS_FEATURE_LEVEL = D3D_FEATURE_LEVEL_12_0;

// number of RTV descriptors to allocate in the heap
constexpr uint32_t RTV_DESCRIPTOR_COUNT{256};

// count is 2 to accommodate back buffer (one is presenting while the other is a back buffer)
constexpr uint32_t SWAP_CHAIN_BUFFER_COUNT{2};

class CommandList10;

/**
 * Device class encapsulates the D3D12 device and related resources.
 */
class Device {
    // Alias for Microsoft::WRL::ComPtr
    template <typename T>
    using ComPtr = Microsoft::WRL::ComPtr<T>;

   public:
    /**
     *
     * @param FeatureLevel
     * @param IsHardwareDevice
     * @param HasMaxVideoMemory
     * @param OutDevice
     * @return
     */
    static bool Create(D3D_FEATURE_LEVEL FeatureLevel,
                       bool IsHardwareDevice,
                       bool HasMaxVideoMemory,
                       std::unique_ptr<Device>& OutDevice);

    static bool CreateCommandAllocator(ComPtr<ID3D12Device14>& mD3DDevice,
                                       D3D12_COMMAND_LIST_TYPE Type,
                                       D3D12_COMMAND_LIST_FLAGS Flags,
                                       std::unique_ptr<CommandAllocator>& OutAllocator);

    static bool CreateCommandQueue(ComPtr<ID3D12Device14>& mD3DDevice,
                                   D3D12_COMMAND_LIST_TYPE Type,
                                   D3D12_COMMAND_QUEUE_PRIORITY Priority,
                                   D3D12_COMMAND_QUEUE_FLAGS QueueFlags,
                                   D3D12_FENCE_FLAGS FenceFlags,
                                   std::unique_ptr<CommandQueue>& OutQueue);

    static bool CreateDescriptorHeap(ComPtr<ID3D12Device14>& mD3DDevice,
                                     D3D12_DESCRIPTOR_HEAP_TYPE Type,
                                     uint32_t Count,
                                     std::unique_ptr<DescriptorHeap>& OutHeap);

    static bool GetBestDevice(ComPtr<IDXGIFactory7>& DXGIFactory,
                              D3D_FEATURE_LEVEL FeatureLevel,
                              bool IsHardwareDevice,
                              bool HasMaxVideoMemory,
                              ComPtr<ID3D12Device14>& OutD3DDevice);

    Device(std::unique_ptr<DebugLayer>&& DebugLayer,
           std::unique_ptr<DescriptorHeap> rtvHeap,
           std::unique_ptr<CommandQueue>&& CommandQueue,
           std::unique_ptr<CommandAllocator>&& CommandAllocator,
           ComPtr<IDXGIFactory7>&& DXGIFactory,
           ComPtr<ID3D12Device14> D3DDevice)
        : mDebugLayer{std::move(DebugLayer)},

          mCommandQueue{std::move(CommandQueue)},
          mCommandAllocator{std::move(CommandAllocator)},

          mRTVHeap{std::move(rtvHeap)},
          mDXGIFactory{std::move(DXGIFactory)},
          mD3DDevice{std::move(D3DDevice)} {}

    ~Device() {
        LOG_INFO(L"\t\tFreeing Device.\n");
    }

    // Deleted copy constructor and assignment operator to prevent copying
    Device(Device& copy) = delete;
    Device& operator=(const Device& copy) = delete;

    // Instance members
    bool CreatePipelineState(D3D12_GRAPHICS_PIPELINE_STATE_DESC& Desc,
                             std::unique_ptr<PipelineState>& OutPSO) const;

    void CreateRenderTargetView(ID3D12Resource2* pResource,
                                D3D12_RENDER_TARGET_VIEW_DESC& desc,
                                D3D12_CPU_DESCRIPTOR_HANDLE& CpuHandle) const;

    bool CreateRootSignature(Bytes& Source, std::unique_ptr<RootSignature>& OutRootSignature) const;

    bool CreateSwapChain(HWND hWnd,
                         uint32_t Width,
                         uint32_t Height,
                         std::unique_ptr<SwapChain>& OutSwapChain);

    template <typename T>
    bool CreateBuffer(std::wstring Name,
                      D3D12_HEAP_TYPE Type,
                      size_t Size,
                      std::unique_ptr<T>& OutBuffer)
        requires std::is_base_of_v<ByteBuffer, T>
    {
        size_t BufferSize = ByteUtil::AlignTo256Bytes(Size);

        ComPtr<ID3D12Resource2> pD3DBuffer;
        CD3DX12_HEAP_PROPERTIES heapProps{Type};
        CD3DX12_RESOURCE_DESC bufferDesc{CD3DX12_RESOURCE_DESC::Buffer(BufferSize)};
        if (FAILED(mD3DDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE,
                                                       &bufferDesc, D3D12_RESOURCE_STATE_COMMON,
                                                       nullptr, IID_PPV_ARGS(&pD3DBuffer)))) {
            LOG_ERROR(L"\t\tFailed to create buffer.\n");
            return false;
        }
        pD3DBuffer->SetName(Name.c_str());
        OutBuffer = std::make_unique<T>(Type, BufferSize, pD3DBuffer);
        return true;
    }

    bool CreateMesh(uint32_t VertexCount,
                    uint32_t VertexStrideInBytes,
                    const void* Data,
                    std::shared_ptr<Mesh>& OutMesh);

    bool CreateMeshInstance(std::shared_ptr<Mesh> Model,
                            std::unique_ptr<MeshInstance>& OutModelInstance);

    bool GetCommandList(CommandList10& OutCommandList) const;

    bool GetFrameCommandList(SwapChain& SwapChain, FrameCommandList10& OutCommandList) const;

    void DisableAltEnterFullscreenToggle(HWND mHWnd) const {
        // Disable the Alt+Enter fullscreen toggle feature. Switching to fullscreen
        mDXGIFactory->MakeWindowAssociation(mHWnd, DXGI_MWA_NO_ALT_ENTER);
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
