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

    GraphicsContext(std::unique_ptr<DebugLayer>&& DebugLayer)
        : mDebugLayer(std::move(DebugLayer)) {};

    // Deleted copy constructor and assignment operator to prevent copying
    GraphicsContext(GraphicsContext& copy) = delete;
    GraphicsContext& operator=(const GraphicsContext& copy) = delete;

    /**
     * Executes the draw call for rendering.
     *
     * @return true if the draw call was successful; false otherwise.
     */
    bool Draw();

   private:
    // IMPORTANT. Keep the DebugLayer at the very top to ensure it is destroyed last.
    // It reports on LIVE DX objects before the context is destroyed.
    std::unique_ptr<DebugLayer> mDebugLayer;

    std::unique_ptr<Device> mDevice;
};
