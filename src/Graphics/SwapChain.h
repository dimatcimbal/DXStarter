#pragma once

#include "CommandQueue.h"
#include "Includes/ComIncl.h"
#include "Includes/GraphicsIncl.h"

class SwapChain {
   public:
    SwapChain(uint32_t BufferCount,
              DXGI_FORMAT Format,
              uint32_t Flags,
              CommandQueue& GraphicsQueue,
              Microsoft::WRL::ComPtr<IDXGISwapChain1>&& DXGISwapChain)
        : mBufferCount(BufferCount),
          mFormat(Format),
          mFlags(Flags),
          mGraphicsQueue(GraphicsQueue),
          mDXGISwapChain(std::move(DXGISwapChain)) {}

    // Prohibit copying
    SwapChain(const SwapChain& copy) = delete;
    SwapChain& operator=(const SwapChain&) = delete;

    // Accessors
    uint32_t GetBufferCount() const {
        return mBufferCount;
    }

    // member functions
    bool Resize(uint32_t Width, uint32_t Height) const;
    bool Present() const;
    bool FlushAll() const;

   private:
    uint32_t mBufferCount;
    DXGI_FORMAT mFormat;
    uint32_t mFlags;

    CommandQueue& mGraphicsQueue;
    Microsoft::WRL::ComPtr<IDXGISwapChain1> mDXGISwapChain;
};
