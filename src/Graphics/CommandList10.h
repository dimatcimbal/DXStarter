#pragma once

#include <utility>

#include "CommandQueue.h"
#include "Includes/ComIncl.h"
#include "Includes/GraphicsIncl.h"
#include "Logging/Logging.h"
#include "Resources/Resource.h"
#include "SwapChain.h"

/**
 * CommandList is a RAII wrapper for any ID3D12GraphicsCommandList version.
 * It ensures that the command list is executed and the command queue is waited on when it goes
 * out of scope.
 */
class CommandList10 {
   public:
    CommandList10() = default;

    CommandList10(CommandQueue* CommandQueue, ID3D12GraphicsCommandList10* CommandList)
        : mCommandQueue{CommandQueue}, mD3DCommandList{CommandList} {}

    ~CommandList10() {
        // Execute and wait on current command list if valid
        if (mCommandQueue && mD3DCommandList) {
            if (!mCommandQueue->ExecuteCommandList(mD3DCommandList)) {
                LOG_ERROR(L"\tFailed to execute command list.\n");
            }
            if (!mCommandQueue->WaitForIdle()) {
                LOG_ERROR(L"\tFailed to wait on command queue.\n");
            }
        }
    }

    // Prohibit copying
    CommandList10(const CommandList10& copy) = delete;
    CommandList10& operator=(const CommandList10& copy) = delete;

    // Move constructor
    CommandList10(CommandList10&& other) noexcept
        : mCommandQueue{std::exchange(other.mCommandQueue, nullptr)},
          mD3DCommandList{std::exchange(other.mD3DCommandList, nullptr)} {}

    // Move assignment operator
    CommandList10& operator=(CommandList10&& other) noexcept {
        if (this != &other) {
            // The command list doesn't own these resources, so just move the pointers
            mCommandQueue = std::exchange(other.mCommandQueue, nullptr);
            mD3DCommandList = std::exchange(other.mD3DCommandList, nullptr);
        }
        return *this;
    }

    // Instance members
    void CopyBufferRegion(const Resource& From,
                          size_t FromOffset,
                          const Resource& To,
                          size_t NumBytes) const {
        mD3DCommandList->CopyBufferRegion(To.GetResource(), 0, From.GetResource(), FromOffset,
                                          NumBytes);
    }

    void ClearTarget(D3D12_CPU_DESCRIPTOR_HANDLE RTV, const float ColorRGBA[4]) const {
        mD3DCommandList->ClearRenderTargetView(RTV, ColorRGBA, 0, nullptr);
    }

    void SetRenderTarget(D3D12_CPU_DESCRIPTOR_HANDLE RTV) const {
        SetRenderTargets(1, &RTV);
    }

    void SetRenderTargets(uint32_t count, D3D12_CPU_DESCRIPTOR_HANDLE RTVs[]) const {
        mD3DCommandList->OMSetRenderTargets(count, RTVs, FALSE, nullptr);
    }

    void SetVertexBuffer(uint32_t Slot, const D3D12_VERTEX_BUFFER_VIEW& VBView) const {
        mD3DCommandList->IASetVertexBuffers(Slot, 1, &VBView);
    }

    void SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY Topology) const {
        mD3DCommandList->IASetPrimitiveTopology(Topology);
    }

    void DrawInstanced(uint32_t NumVertexPerInstance, uint32_t StartVertexOffset) const {
        DrawInstanced(NumVertexPerInstance, 1, StartVertexOffset, 0);
    }

    void DrawInstanced(uint32_t NumVertexPerInstance,
                       uint32_t NumInstance,
                       uint32_t StartVertexOffset,
                       uint32_t StartInstanceOffset) const {
        FlushResourceBarriers();
        mD3DCommandList->DrawInstanced(NumVertexPerInstance, NumInstance, StartVertexOffset,
                                       StartInstanceOffset);
    }

    void FlushResourceBarriers() const {}

    /** Transition a resource from one state to another.
     *  This is a template method that accepts any type derived from Resource.
     */
    template <typename T>
    void TransitionResource(T& Rsrc, D3D12_RESOURCE_STATES Before, D3D12_RESOURCE_STATES After)
        requires std::is_base_of_v<Resource, T>
    {
        // Prepare a resource barrier for transition.
        D3D12_RESOURCE_BARRIER barrier{};

        // Configure this as a transition barrier.
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;

        // The resource to transition.
        barrier.Transition.pResource = Rsrc.GetResource();
        Rsrc.SetCurrentState(After);

        // Transition only the first subresource (subresource index 0).
        barrier.Transition.Subresource = 0;

        barrier.Transition.StateBefore = Before;
        barrier.Transition.StateAfter = After;

        mD3DCommandList->ResourceBarrier(1, &barrier);
    }

    ID3D12GraphicsCommandList10* operator->() const {
        return mD3DCommandList;
    }

   protected:
    CommandQueue* mCommandQueue{nullptr};
    ID3D12GraphicsCommandList10* mD3DCommandList{nullptr};
};

/**
 * FrameCommandList is a RAII wrapper for any ID3D12GraphicsCommandList version that is used for
 * rendering a frame. It ensures that the frame is begun and ended on the swap chain, and that the
 * command list is executed and the command queue is waited on when it goes out of scope.
 */
class FrameCommandList10 : public CommandList10 {
    friend Device;

   public:
    FrameCommandList10() = default;

    FrameCommandList10(SwapChain* mSwapChain,
                       CommandQueue* CommandQueue,
                       ID3D12GraphicsCommandList10* CommandList)
        : CommandList10(CommandQueue, CommandList), mSwapChain{mSwapChain} {
        // Begin the frame on the swap chain
        mSwapChain->BeginFrame(*this);
    }

    ~FrameCommandList10() {
        // End the frame on the swap chain
        if (mSwapChain) {
            mSwapChain->EndFrame(*this);
        }

        // Let the base class handle executing and waiting on the command list
        // CommandList10::~CommandList10(); is implicitly called
    }

    // Prohibit copying
    FrameCommandList10(const FrameCommandList10& copy) = delete;
    FrameCommandList10& operator=(const FrameCommandList10& copy) = delete;

    // Move constructor
    FrameCommandList10(FrameCommandList10&& other) noexcept
        : CommandList10(std::move(other)), mSwapChain{std::exchange(other.mSwapChain, nullptr)} {}

    // Move assignment operator
    FrameCommandList10& operator=(FrameCommandList10&& other) noexcept {
        if (this != &other) {
            // The command list doesn't own these resources, so just move the pointers
            mSwapChain = std::exchange(other.mSwapChain, nullptr);
            CommandList10::operator=(std::move(other));
        }
        return *this;
    }

   private:
    SwapChain* mSwapChain{nullptr};
};
