#pragma once
#include <memory>

#include "CommandList.h"
#include "DebugLayer.h"
#include "Device.h"
#include "SwapChain.h"

// Graphics configs below
constexpr D3D_FEATURE_LEVEL GRAPHICS_FEATURE_LEVEL = D3D_FEATURE_LEVEL_12_0;

// number of RTV descriptors to allocate in the heap
constexpr uint32_t RTV_DESCRIPTOR_COUNT{256};

// count is 2 to accommodate back buffer (one is presenting while the other is a back buffer)
constexpr uint32_t SWAP_CHAIN_BUFFER_COUNT{2};

/**
 * GraphicsContext manages the DirectX graphics pipeline.
 * It initializes and maintains the graphics context, including debug layers.
 */
class GraphicsContext {
   public:
    /**
     * Factory method to create a GraphicsContext instance.
     *
     * @param OutContext A unique pointer to hold the created GraphicsContext instance.
     * @return true if the GraphicsContext was successfully created; false otherwise.
     */
    static bool Create(std::unique_ptr<GraphicsContext>& OutContext);

    GraphicsContext(std::unique_ptr<CommandQueue>&& CommandQueue,
                    std::unique_ptr<CommandAllocator>&& CommandAllocator,
                    std::unique_ptr<DescriptorHeap> RTVHeap,
                    std::unique_ptr<Device> Device,
                    std::unique_ptr<DebugLayer>&& DebugLayer)
        : mCommandQueue(std::move(CommandQueue)),
          mCommandAllocator(std::move(CommandAllocator)),
          mRTVHeap(std::move(RTVHeap)),
          mDevice(std::move(Device)),
          mDebugLayer(std::move(DebugLayer)) {}

    ~GraphicsContext() {
        LOG_INFO(L"\tFreeing GraphicsContext.\n");

        // Flush the command queue for all the buffers in the swap chain to ensure all GPU
        // operations are complete before destruction.
        FlushAll();
    }

    // Prohibit copying
    GraphicsContext(GraphicsContext& copy) = delete;
    GraphicsContext& operator=(const GraphicsContext& copy) = delete;

    /**
     * Returns a command list for recording graphics commands. The command list is RAII wrapped,
     * closes itself and executes when goes out of scope.
     */
    bool GetCommandList(CommandList<ID3D12GraphicsCommandList10>& OutCommandList) const;
    bool Present() const;

    // Window event handlers
    bool CreateSwapChain(HWND hWnd, uint32_t Width = 320, uint32_t Height = 240);
    bool ResizeSwapChain(uint32_t Width, uint32_t Height);

   private:
    bool FlushAll() const;

   private:
    // IMPORTANT! Keep the DebugLayer at the very top to ensure it is destroyed the last.
    // It reports on LIVE DX objects before the context is destroyed.
    std::unique_ptr<DebugLayer> mDebugLayer;

    // The rest of the owned resources
    std::unique_ptr<SwapChain> mSwapChain;
    std::unique_ptr<DescriptorHeap> mRTVHeap;
    std::unique_ptr<CommandQueue> mCommandQueue;
    std::unique_ptr<CommandAllocator> mCommandAllocator;
    std::unique_ptr<Device> mDevice;

    // Display resolution
    uint32_t mWidth{0};
    uint32_t mHeight{0};
};
