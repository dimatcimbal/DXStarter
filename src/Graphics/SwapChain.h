#pragma once

#include <vector>

#include "ColorBuffer.h"
#include "Includes/ComIncl.h"
#include "Includes/GraphicsIncl.h"

class Device;
class DescriptorHeap;
class CommandQueue;
class CommandList10;

class SwapChain {
    // Alias for Microsoft::WRL::ComPtr
    template <typename T>
    using ComPtr = Microsoft::WRL::ComPtr<T>;

   public:
    SwapChain(uint32_t BufferCount,
              DXGI_FORMAT Format,
              uint32_t Flags,
              Device& pDevice,
              CommandQueue& GraphicsQueue,
              ComPtr<IDXGISwapChain4>&& DXGISwapChain)
        : mBackBufferCount{BufferCount},
          mFormat{Format},
          mFlags{Flags},
          mDevice(pDevice),
          mGraphicsQueue{GraphicsQueue},
          mDXGISwapChain{std::move(DXGISwapChain)},
          mBackBuffers(BufferCount) {}

    ~SwapChain() {
        // Flush the command queue for all the buffers in the swap chain to ensure all GPU
        // operations are complete before destruction.
        FlushAll();
    }

    // Prohibit copying
    SwapChain(const SwapChain& copy) = delete;
    SwapChain& operator=(const SwapChain&) = delete;

    // Accessors
    uint32_t GetBufferCount() const {
        return mBackBufferCount;
    }

    ColorBuffer* GetCurrentBackBuffer() const {
        return mBackBuffers[mCurrentBackBufferIndex].get();
    }

    // member functions
    bool FlushAll();
    bool Resize(uint32_t Width, uint32_t Height);
    bool Present() const;

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
    bool BuffersReadTo(std::vector<std::unique_ptr<ColorBuffer>>& OutVector) const;

    uint32_t mBackBufferCount;
    uint32_t mCurrentBackBufferIndex{0};
    DXGI_FORMAT mFormat;
    uint32_t mFlags;

    Device& mDevice;
    CommandQueue& mGraphicsQueue;
    ComPtr<IDXGISwapChain4> mDXGISwapChain;

    // IMPORTANT! Keep the size of the vector intact as mBackBufferCount,
    // i.e. avoid clear, push_back, etc.
    std::vector<std::unique_ptr<ColorBuffer>> mBackBuffers;
};
