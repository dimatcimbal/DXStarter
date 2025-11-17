#pragma once

#include <utility>

#include "CommandQueue.h"
#include "Includes/ComIncl.h"
#include "Includes/GraphicsIncl.h"
#include "Logging/Logging.h"
#include "Mesh/Mesh.h"
#include "Resource/DeviceBuffer.h"
#include "Resource/Resource.h"
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

    virtual ~CommandList10() {
        // Execute and wait on current command list if valid
        if (mCommandQueue && mD3DCommandList) {
            if (!mCommandQueue->ExecuteCommandList(mD3DCommandList)) {
                LOG_ERROR(L"Failed to execute command list.\n");
            }
            if (!mCommandQueue->WaitForIdle()) {
                LOG_ERROR(L"Failed to wait on command queue.\n");
            }
        }
    }

    // Prohibit copying
    CommandList10(const CommandList10& Copy) = delete;
    CommandList10& operator=(const CommandList10& Copy) = delete;

    // Move constructor
    CommandList10(CommandList10&& Other) noexcept
        : mCommandQueue{std::exchange(Other.mCommandQueue, nullptr)},
          mD3DCommandList{std::exchange(Other.mD3DCommandList, nullptr)} {}

    // Move assignment operator
    CommandList10& operator=(CommandList10&& Other) noexcept {
        if (this != &Other) {
            // The command list doesn't own these resources, so just move the pointers
            mCommandQueue = std::exchange(Other.mCommandQueue, nullptr);
            mD3DCommandList = std::exchange(Other.mD3DCommandList, nullptr);
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

    void SetConstantBuffer(uint32_t Index, DeviceBuffer& View) const {
        mD3DCommandList->SetGraphicsRootConstantBufferView(Index, View.GetDeviceVirtualAddress());
    }

    void SetRenderTarget(ColorBuffer& RTV) const {
        D3D12_CPU_DESCRIPTOR_HANDLE View = RTV.GetRTV();
        mD3DCommandList->OMSetRenderTargets(1, &View, FALSE, nullptr);
    }

    void SetVertexBuffer(uint32_t Slot, const Mesh& Mesh) const {
        D3D12_VERTEX_BUFFER_VIEW vbv;
        vbv.BufferLocation = Mesh.GetVertexBuffer();
        vbv.SizeInBytes = Mesh.GetVertexBufferSize();
        vbv.StrideInBytes = Mesh.GetStrideInBytes();

        mD3DCommandList->IASetVertexBuffers(Slot, 1, &vbv);
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
    void TransitionResource(T& Rsrc, D3D12_RESOURCE_STATES After)
        requires std::is_base_of_v<Resource, T>
    {
        D3D12_RESOURCE_STATES Before = Rsrc.GetCurrentState();

        if (Before != After) {
            // Prepare a transition barrier desc.
            D3D12_RESOURCE_BARRIER desc{};
            desc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            desc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            // The resource to transition.
            desc.Transition.pResource = Rsrc.GetResource();
            // Transition all subresources
            desc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
            desc.Transition.StateBefore = Before;
            desc.Transition.StateAfter = After;

            Rsrc.SetCurrentState(After);

            mD3DCommandList->ResourceBarrier(1, &desc);
        }

        // TODO: Implement Split Barriers in another class
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
   public:
    FrameCommandList10() = default;

    FrameCommandList10(SwapChain* SwapChain,
                       CommandQueue* CommandQueue,
                       ID3D12GraphicsCommandList10* CommandList)
        : CommandList10(CommandQueue, CommandList), mSwapChain{SwapChain} {
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
    FrameCommandList10(const FrameCommandList10& Copy) = delete;
    FrameCommandList10& operator=(const FrameCommandList10& Copy) = delete;

    // Move constructor
    FrameCommandList10(FrameCommandList10&& Other) noexcept
        : CommandList10(std::move(Other)), mSwapChain{std::exchange(Other.mSwapChain, nullptr)} {}

    // Move assignment operator
    FrameCommandList10& operator=(FrameCommandList10&& Other) noexcept {
        if (this != &Other) {
            // The command list doesn't own these resources, so just move the pointers
            mSwapChain = std::exchange(Other.mSwapChain, nullptr);
            CommandList10::operator=(std::move(Other));
        }
        return *this;
    }

    void ClearRenderTarget(const float* ClearColorRGBA) const {
        // The SwapChain should be set
        if (!mSwapChain) {
            LOG_ERROR(L"ClearRenderTarget: mSwapChain is nullptr.");
            return;
        }
        D3D12_CPU_DESCRIPTOR_HANDLE RenderTarget = mSwapChain->GetCurrentBackBuffer().GetRTV();
        mD3DCommandList->ClearRenderTargetView(RenderTarget, ClearColorRGBA, 0, nullptr);
    }

   private:
    friend class Device;
    SwapChain* mSwapChain{nullptr};
};
