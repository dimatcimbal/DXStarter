#pragma once

#include <memory>
#include <vector>

#include "Includes/ComIncl.h"
#include "Includes/GraphicsIncl.h"
#include "Resource/ColorBuffer.h"

// Forward declarations
class Device;
class DescriptorHeap;
class CommandQueue;
class FrameCommandList10;

/**
 * SwapChain class encapsulates a DXGI swap chain and its associated back buffers.
 */
class SwapChain {
    // Alias for Microsoft::WRL::ComPtr
    template <typename T>
    using ComPtr = Microsoft::WRL::ComPtr<T>;

   public:
    SwapChain(uint32_t BufferCount,
              DXGI_FORMAT Format,
              uint32_t Flags,
              Device& Device,
              CommandQueue& GraphicsQueue,
              ComPtr<IDXGISwapChain4>&& DXGISwapChain)
        : mBackBufferCount{BufferCount},
          mFormat{Format},
          mFlags{Flags},
          mDevice(&Device),
          mGraphicsQueue{&GraphicsQueue},
          mDXGISwapChain{std::move(DXGISwapChain)},
          mBackBuffers(BufferCount) {}

    ~SwapChain() {
        // Flush the command queue for all the buffers in the swap chain to ensure all GPU
        // operations are complete before destruction.
        FlushAll();
    }

    // Prohibit copying
    SwapChain(const SwapChain& Copy) = delete;
    SwapChain& operator=(const SwapChain&) = delete;

    // Accessors
    uint32_t GetBufferCount() const {
        return mBackBufferCount;
    }

    ColorBuffer& GetCurrentBackBuffer() const {
        return *mBackBuffers[mCurrentBackBufferIndex];
    }

    // member functions
    bool FlushAll();
    bool Resize(uint32_t Width, uint32_t Height);
    bool Present() const;

    /**
     * Transition the current back buffer to a render target state at the beginning of the frame.
     * @param Cmdl Command list to record the resource barrier commands into.
     */
    void BeginFrame(FrameCommandList10& Cmdl);

    /**
     * Transition the current back buffer to a present state at the end of the frame.
     * @param Cmdl Command list to record the resource barrier commands into.
     */
    void EndFrame(FrameCommandList10& Cmdl) const;

   private:
    /**
     * Retrieves the buffers from the DXGI swap chain and stores them in mBuffers.
     * @returns true if successful, false otherwise.
     */
    bool BuffersReadTo(std::vector<std::unique_ptr<ColorBuffer>>& OutVector) const;

    // Non-owned
    CommandQueue* mGraphicsQueue;
    Device* mDevice;

    // Owned
    ComPtr<IDXGISwapChain4> mDXGISwapChain;
    // IMPORTANT! Keep the size of the vector intact as mBackBufferCount,
    // i.e. avoid clear, push_back, etc.
    std::vector<std::unique_ptr<ColorBuffer>> mBackBuffers;

    uint32_t mBackBufferCount;
    uint32_t mCurrentBackBufferIndex{0};
    DXGI_FORMAT mFormat;
    uint32_t mFlags;
};
