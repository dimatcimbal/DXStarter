#pragma once
#include <wrl/client.h>

#include <memory>
#include <mutex>

#include "Includes/GraphicsIncl.h"
#include "Logging/Logging.h"

class Device;

class CommandQueue {
   public:
    CommandQueue(D3D12_COMMAND_LIST_TYPE Type, uint64_t InitFenceValue, HANDLE FenceEventHandle,
                 Microsoft::WRL::ComPtr<ID3D12Fence1>&& D3D12Fence,
                 Microsoft::WRL::ComPtr<ID3D12CommandQueue>&& D3D12CommandQueue)
        : mType(Type),
          mNextFenceValue(InitFenceValue),
          mFenceEventHandle(FenceEventHandle),
          mD3D12Fence(std::move(D3D12Fence)),
          mD3D12CommandQueue(std::move(D3D12CommandQueue)) {}
    ~CommandQueue() {
        LOG_INFO(L"Freeing CommandQueue.\n");
    };

    // Class members
    bool ExecuteCommandList(ID3D12GraphicsCommandList10* CommandList);
    bool WaitForFenceValue(uint64_t FenceValueToWait);
    bool WaitForIdle() {
        return WaitForFenceValue(NextFenceValue());
    }

    // Prohibit copying
    CommandQueue(const CommandQueue& copy) = delete;
    CommandQueue& operator=(const CommandQueue& copy) = delete;

   private:
    uint64_t NextFenceValue();

   private:
    D3D12_COMMAND_LIST_TYPE mType;
    uint64_t mNextFenceValue;
    HANDLE mFenceEventHandle;

    // synchronization
    std::mutex mFenceValueMutex;
    std::mutex mFenceEventMutex;

    Microsoft::WRL::ComPtr<ID3D12Fence1> mD3D12Fence;
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> mD3D12CommandQueue;
};
