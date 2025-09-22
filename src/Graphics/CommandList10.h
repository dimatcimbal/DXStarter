#pragma once

#include "CommandQueue.h"
#include "Includes/GraphicsIncl.h"
#include "Logging/Logging.h"
#include "SwapChain.h"

/**
 * CommandList is a RAII wrapper for any ID3D12GraphicsCommandList version.
 * It ensures that the command list is executed and the command queue is waited on when it goes
 * out of scope.
 */
class CommandList10 {
   public:
    CommandList10() = default;

    CommandList10(CommandQueue* CommandQueue,
                  SwapChain* mSwapChain,
                  ID3D12GraphicsCommandList10* CommandList)
        : mCommandQueue{CommandQueue}, mSwapChain{mSwapChain}, mCommandList{CommandList} {
        // Begin the frame on the swap chain
        mSwapChain->BeginFrame(*this);
    }

    ~CommandList10() {
        // End the frame on the swap chain
        if (mSwapChain) {
            mSwapChain->EndFrame(*this);
        }

        // Execute and wait on current command list if valid
        if (mCommandQueue && mCommandList) {
            if (!mCommandQueue->ExecuteCommandList(mCommandList)) {
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
        : mSwapChain{other.mSwapChain},
          mCommandQueue{other.mCommandQueue},
          mCommandList{other.mCommandList} {
        other.mSwapChain = nullptr;
        other.mCommandQueue = nullptr;
        other.mCommandList = nullptr;
    }

    // Move assignment operator
    CommandList10& operator=(CommandList10&& other) noexcept {
        if (this != &other) {
            // The command list doesn't own these resources, so just move the pointers
            mSwapChain = other.mSwapChain;
            mCommandQueue = other.mCommandQueue;
            mCommandList = other.mCommandList;
            other.mSwapChain = nullptr;
            other.mCommandQueue = nullptr;
            other.mCommandList = nullptr;
        }
        return *this;
    }

    // Instance members
    void ClearTarget(D3D12_CPU_DESCRIPTOR_HANDLE RTV, const float ColorRGBA[4]) const {
        mCommandList->ClearRenderTargetView(RTV, ColorRGBA, 0, nullptr);
    }

    void SetRenderTarget(D3D12_CPU_DESCRIPTOR_HANDLE RTV) const {
        SetRenderTargets(1, &RTV);
    }

    void SetRenderTargets(uint32_t count, D3D12_CPU_DESCRIPTOR_HANDLE RTVs[]) const {
        mCommandList->OMSetRenderTargets(count, RTVs, FALSE, nullptr);
    }

    void TransitionResource(ID3D12Resource2* Resource,
                            D3D12_RESOURCE_STATES Before,
                            D3D12_RESOURCE_STATES After) const;

   private:
    SwapChain* mSwapChain{nullptr};
    CommandQueue* mCommandQueue{nullptr};
    ID3D12GraphicsCommandList10* mCommandList{nullptr};
};
