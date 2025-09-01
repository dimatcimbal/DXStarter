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
     * Factory method to create a GraphicsContext instance.
     *
     * @param OutContext A unique pointer to hold the created GraphicsContext instance.
     * @return true if the GraphicsContext was successfully created; false otherwise.
     */
    static bool Create(std::unique_ptr<GraphicsContext>& OutContext);

    GraphicsContext(std::unique_ptr<CommandQueue>&& CommandQueue, std::unique_ptr<Device> Device,
                    std::unique_ptr<DebugLayer>&& DebugLayer)
        : mCommandQueue(std::move(CommandQueue)),
          mDevice(std::move(Device)),
          mDebugLayer(std::move(DebugLayer)) {};

    ~GraphicsContext() {
        LOG_INFO(L"Freeing GraphicsContext.\n");
    }

    // Deleted copy constructor and assignment operator to prevent copying
    GraphicsContext(GraphicsContext& copy) = delete;
    GraphicsContext& operator=(const GraphicsContext& copy) = delete;

    /**
     * Executes the draw call for rendering.
     *
     * @return true if the draw call was successful; false otherwise.
     */
    bool Draw();

    // Window event handlers
    bool OnWindowCreate(HWND hWnd, uint32_t Width = 320, uint32_t Height = 240);
    bool OnWindowResize(uint32_t Width, uint32_t Height);

   private:
    // IMPORTANT. Keep the DebugLayer at the very top to ensure it is destroyed last.
    // It reports on LIVE DX objects before the context is destroyed.
    std::unique_ptr<DebugLayer> mDebugLayer;

    std::unique_ptr<CommandQueue> mCommandQueue;
    std::unique_ptr<Device> mDevice;
};
