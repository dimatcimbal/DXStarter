#include "DebugLayer.h"

#include "Logging/Logging.h"

bool DebugLayer::Create(std::unique_ptr<DebugLayer>& OutDebugLayer) {
#ifdef _DEBUG
    // Enable the D3D12 debug layer
    Microsoft::WRL::ComPtr<ID3D12Debug6> d3dDebug;
    if (FAILED(D3D12GetDebugInterface(IID_PPV_ARGS(&d3dDebug)))) {
        LOG_ERROR(L"Failed to get D3D12 debug interface.\n");
        return false;
    }
    d3dDebug->EnableDebugLayer();

    // Enable the DXGI debug layer
    Microsoft::WRL::ComPtr<IDXGIDebug1> dxgiDebug;
    if (FAILED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug)))) {
        LOG_ERROR(L"Failed to get DXGI debug interface.\n");
        return false;
    }

    OutDebugLayer = std::make_unique<DebugLayer>(std::move(d3dDebug), std::move(dxgiDebug));
    return true;
#else
    OutDebugLayer = std::make_unique<DebugLayer>();
    return true;
#endif
}

void DebugLayer::ReportLiveObjects() {
#ifdef _DEBUG
    if (mDXGIDebug) {
        LOG_INFO(L"Reporting LIVE D3D12 objects:\n");

        // Report in detail but ignore internal objects
        DXGI_DEBUG_RLO_FLAGS DebugFlags = static_cast<DXGI_DEBUG_RLO_FLAGS>(
            DXGI_DEBUG_RLO_DETAIL | DXGI_DEBUG_RLO_IGNORE_INTERNAL);

        if (FAILED(mDXGIDebug->ReportLiveObjects(
                // Report all live objects
                DXGI_DEBUG_ALL,
                // with the specified flags
                DebugFlags))) {
            LOG_ERROR(L"Failed to report live objects.\n");
        }
    } else {
        LOG_INFO(L"DXGI Debug interface is not initialized.\n");
    }
#endif
}