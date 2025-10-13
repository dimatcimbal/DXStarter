#pragma once

#include <mutex>

#include "Includes/ComIncl.h"
#include "Includes/GraphicsIncl.h"
#include "Logging/Logging.h"

class CommandQueue {
    // Alias for Microsoft::WRL::ComPtr
    template <typename T>
    using ComPtr = Microsoft::WRL::ComPtr<T>;

   public:
    CommandQueue(D3D12_COMMAND_LIST_TYPE Type,
                 uint64_t InitFenceValue,
                 HANDLE FenceEventHandle,
                 ComPtr<ID3D12Fence1>&& D3D12Fence,
                 ComPtr<ID3D12CommandQueue>&& D3D12CommandQueue)
        : mType{Type},
          mNextFenceValue{InitFenceValue},
          mFenceEventHandle{FenceEventHandle},
          mD3D12Fence{std::move(D3D12Fence)},
          mD3D12CommandQueue{std::move(D3D12CommandQueue)} {}
    ~CommandQueue() {
        LOG_INFO(L"\t\tFreeing CommandQueue of type %d\n", mType);
    }

    // Class members
    bool ExecuteCommandList(ID3D12GraphicsCommandList10* CommandList);
    bool WaitForFenceValue(uint64_t FenceValueToWait);
    bool WaitForIdle() {
        return WaitForFenceValue(NextFenceValue());
    }

    // Prohibit copying
    CommandQueue(const CommandQueue& copy) = delete;
    CommandQueue& operator=(const CommandQueue& copy) = delete;

    // Accessors
    ID3D12CommandQueue* GetD3D12CommandQueue() const {
        return mD3D12CommandQueue.Get();
    }

   private:
    uint64_t NextFenceValue();

   private:
    uint64_t mNextFenceValue;
    std::mutex mFenceValueMutex;

    HANDLE mFenceEventHandle;
    std::mutex mFenceEventMutex;

    D3D12_COMMAND_LIST_TYPE mType;
    ComPtr<ID3D12Fence1> mD3D12Fence;
    ComPtr<ID3D12CommandQueue> mD3D12CommandQueue;
};
