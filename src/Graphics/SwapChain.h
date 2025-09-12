#pragma once

#include <vector>

#include "CommandQueue.h"
#include "Includes/ComIncl.h"
#include "Includes/GraphicsIncl.h"

class SwapChain {
    // Alias for Microsoft::WRL::ComPtr
    template <typename T>
    using ComPtr = Microsoft::WRL::ComPtr<T>;

   public:
    SwapChain(uint32_t BufferCount,
              DXGI_FORMAT Format,
              uint32_t Flags,
              CommandQueue& GraphicsQueue,
              ComPtr<IDXGISwapChain1>&& DXGISwapChain)
        : mSwapChainBufferCount{BufferCount},
          mFormat{Format},
          mFlags{Flags},
          mGraphicsQueue{GraphicsQueue},
          mDXGISwapChain{std::move(DXGISwapChain)},
          mSwapChainBuffers(BufferCount) {}

    // Prohibit copying
    SwapChain(const SwapChain& copy) = delete;
    SwapChain& operator=(const SwapChain&) = delete;

    // Accessors
    uint32_t GetBufferCount() const {
        return mSwapChainBufferCount;
    }

    // member functions
    bool FlushAll();
    bool Resize(uint32_t Width, uint32_t Height);
    bool Present() const;

   private:
    /**
     * Retrieves the buffers from the DXGI swap chain and stores them in mBuffers.
     * @returns true if successful, false otherwise.
     */
    bool BuffersReadTo(std::vector<ComPtr<ID3D12Resource2>>& OutVector) const;
    void BuffersRelease();

    uint32_t mSwapChainBufferCount;
    DXGI_FORMAT mFormat;
    uint32_t mFlags;

    CommandQueue& mGraphicsQueue;
    ComPtr<IDXGISwapChain1> mDXGISwapChain;
    std::vector<ComPtr<ID3D12Resource2>> mSwapChainBuffers;
};
