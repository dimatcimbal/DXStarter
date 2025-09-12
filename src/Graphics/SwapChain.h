#pragma once

#include <vector>

#include "CommandQueue.h"
#include "Includes/ComIncl.h"
#include "Includes/GraphicsIncl.h"

class CommandList10;

class SwapChain {
    // Alias for Microsoft::WRL::ComPtr
    template <typename T>
    using ComPtr = Microsoft::WRL::ComPtr<T>;

   public:
    SwapChain(uint32_t BufferCount,
              DXGI_FORMAT Format,
              uint32_t Flags,
              CommandQueue& GraphicsQueue,
              ComPtr<IDXGISwapChain4>&& DXGISwapChain)
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
    bool Present();

    /**
     * Transition the current back buffer to a render target state at the beginning of the frame.
     * @param Cmd Command list to record the resource barrier commands into.
     */
    void BeginFrame(CommandList10& Cmd);

    /**
     * Transition the current back buffer to a present state at the end of the frame.
     * @param Cmd Command list to record the resource barrier commands into.
     */
    void EndFrame(CommandList10& Cmd);

   private:
    /**
     * Retrieves the buffers from the DXGI swap chain and stores them in mBuffers.
     * @returns true if successful, false otherwise.
     */
    bool BuffersReadTo(std::vector<ComPtr<ID3D12Resource2>>& OutVector) const;
    void BuffersRelease();

    uint32_t mSwapChainBufferCount;
    uint32_t mCurrentBackBufferIndex{0};
    DXGI_FORMAT mFormat;
    uint32_t mFlags;

    CommandQueue& mGraphicsQueue;
    ComPtr<IDXGISwapChain4> mDXGISwapChain;
    std::vector<ComPtr<ID3D12Resource2>> mSwapChainBuffers;
};
