#pragma once

#include <wrl/client.h>

#include <memory>

#include "Includes/GraphicsIncl.h"
#include "Logging/Logging.h"

/**
 * DebugLayer is responsible for enabling and managing DirectX debug layers.
 * It provides functionality to create debug layers and report live DirectX objects.
 */
class DebugLayer {
   public:
    /**
     * Factory method to create a DebugLayer instance.
     *
     * @param OutDebugLayer A unique pointer to hold the created DebugLayer instance.
     * @return true if the DebugLayer was successfully created; false otherwise.
     */
    static bool Create(std::unique_ptr<DebugLayer>& OutDebugLayer);

#ifdef _DEBUG
    DebugLayer(Microsoft::WRL::ComPtr<ID3D12Debug6>&& d3dDebug,
               Microsoft::WRL::ComPtr<IDXGIDebug1>&& dxgiDebug)
        : mD3DDebug(std::move(d3dDebug)), mDXGIDebug(std::move(dxgiDebug)) {};
#else
    DebugLayer() = default;
#endif

    ~DebugLayer() {
        LOG_INFO(L"Freeing DebugLayer.\n");
        ReportLiveObjects();
    };

    // Deleted copy constructor and assignment operator to prevent copying
    DebugLayer(DebugLayer& other) = delete;
    DebugLayer& operator=(const DebugLayer& other) = delete;

    /**
     * Reports live DX objects for debugging purposes.
     */
    void ReportLiveObjects();

#ifdef _DEBUG
   private:
    // DirectX debug layer interface
    Microsoft::WRL::ComPtr<ID3D12Debug6> mD3DDebug;

    // DXGI debug interface
    Microsoft::WRL::ComPtr<IDXGIDebug1> mDXGIDebug;
#endif
};
