#pragma once
#include "CommandAllocator.h"
#include "CommandQueue.h"
#include "DescriptorHeap.h"
#include "Includes/ComIncl.h"
#include "Includes/GraphicsIncl.h"
#include "Logging/Logging.h"
#include "SwapChain.h"

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

    Device(ComPtr<IDXGIFactory7>&& DXGIFactory, ComPtr<ID3D12Device14> D3DDevice)
        : mDXGIFactory{std::move(DXGIFactory)}, mD3DDevice{std::move(D3DDevice)} {};

    ~Device() {
        LOG_INFO(L"\t\tFreeing Device.\n");
    }

    // Class members
    bool CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE Type,
                                D3D12_COMMAND_LIST_FLAGS Flags,
                                std::unique_ptr<CommandAllocator>& OutAllocator) const;

    bool CreateCommandQueue(D3D12_COMMAND_LIST_TYPE Type,
                            D3D12_COMMAND_QUEUE_PRIORITY Priority,
                            D3D12_COMMAND_QUEUE_FLAGS QueueFlags,
                            D3D12_FENCE_FLAGS FenceFlags,
                            std::unique_ptr<CommandQueue>& OutQueue) const;

    bool CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE Type,
                              uint32_t Count,
                              std::unique_ptr<DescriptorHeap>& OutHeap) const;

    bool CreateSwapChain(HWND hWnd,
                         CommandQueue& GraphicsQueue,
                         uint32_t Width,
                         uint32_t Height,
                         uint32_t BufferCount,
                         DXGI_USAGE BufferUsage,
                         DXGI_FORMAT Format,
                         uint32_t Flags,
                         std::unique_ptr<SwapChain>& OutSwapChain) const;

    void DisableAltEnterFullscreenToggle(HWND mHWnd) {
        // Disable the Alt+Enter fullscreen toggle feature. Switching to fullscreen
        mDXGIFactory->MakeWindowAssociation(mHWnd, DXGI_MWA_NO_ALT_ENTER);
    }

    // Deleted copy constructor and assignment operator to prevent copying
    Device(Device& copy) = delete;
    Device& operator=(const Device& copy) = delete;

   private:
    ComPtr<IDXGIFactory7> mDXGIFactory;
    ComPtr<ID3D12Device14> mD3DDevice;
};
