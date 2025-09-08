#pragma once
#include <memory>

#include "DebugLayer.h"
#include "Device.h"

// Graphics configs below
constexpr D3D_FEATURE_LEVEL GRAPHICS_FEATURE_LEVEL = D3D_FEATURE_LEVEL_12_0;

/**
 * GraphicsContext manages the DirectX graphics pipeline.
 * It initializes and maintains the graphics context, including debug layers.
 */
class GraphicsContext {
   public:
    /**
     * CommandList is a RAII wrapper for any ID3D12GraphicsCommandList version.
     * It ensures that the command list is executed and the command queue is waited on when it goes
     * out of scope.
     */
    template <typename TCommandList,
              typename =
                  std::enable_if_t<std::is_base_of<ID3D12GraphicsCommandList, TCommandList>::value>>
    class CommandList {
       public:
        CommandList(CommandQueue* CommandQueue, TCommandList* CommandList)
            : mCommandQueue(CommandQueue), mCommandList(CommandList) {}

        ~CommandList() {
            if (!mCommandQueue->ExecuteCommandList(mCommandList)) {
                LOG_ERROR(L"\tFailed to execute command list.\n");
            }
            if (!mCommandQueue->WaitForIdle()) {
                LOG_ERROR(L"\tFailed to wait on command queue.\n");
            }
        }

        // Prohibit copying
        CommandList(const CommandList& copy) = delete;
        CommandList& operator=(const CommandList& copy) = delete;

        /**
         * Allows CommandList to be used as if it were a pointer to the command list type.
         */
        TCommandList* operator->() const {
            return mCommandList;
        }

       private:
        CommandQueue* mCommandQueue;
        TCommandList* mCommandList;
    };

    /**
     * Factory method to create a GraphicsContext instance.
     *
     * @param OutContext A unique pointer to hold the created GraphicsContext instance.
     * @return true if the GraphicsContext was successfully created; false otherwise.
     */
    static bool Create(std::unique_ptr<GraphicsContext>& OutContext);

    GraphicsContext(std::unique_ptr<CommandQueue>&& CommandQueue,
                    std::unique_ptr<CommandAllocator>&& CommandAllocator,
                    std::unique_ptr<Device> Device,
                    std::unique_ptr<DebugLayer>&& DebugLayer)
        : mCommandQueue(std::move(CommandQueue)),
          mCommandAllocator(std::move(CommandAllocator)),
          mDevice(std::move(Device)),
          mDebugLayer(std::move(DebugLayer)) {}

    ~GraphicsContext() {
        LOG_INFO(L"\tFreeing GraphicsContext.\n");

        // Flush the command queue for all the buffers to ensure all GPU operations
        // are complete before destruction.
        FlushAll();
    }

    // Prohibit copying
    GraphicsContext(GraphicsContext& copy) = delete;
    GraphicsContext& operator=(const GraphicsContext& copy) = delete;

    /**
     * Executes the draw call for rendering.
     *
     * @return true if the draw call was successful; false otherwise.
     */
    bool Draw() const;

    // Window event handlers
    bool CreateSwapChain(HWND hWnd, uint32_t Width = 320, uint32_t Height = 240);
    bool ResizeSwapChain(uint32_t Width, uint32_t Height);

   private:
    bool FlushAll() const;

   private:
    // IMPORTANT. Keep the DebugLayer at the very top to ensure it is destroyed last.
    // It reports on LIVE DX objects before the context is destroyed.
    std::unique_ptr<DebugLayer> mDebugLayer;

    std::unique_ptr<CommandQueue> mCommandQueue;
    std::unique_ptr<CommandAllocator> mCommandAllocator;
    std::unique_ptr<Device> mDevice;
};
