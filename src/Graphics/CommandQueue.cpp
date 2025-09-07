#include "CommandQueue.h"

bool CommandQueue::ExecuteCommandList(ID3D12GraphicsCommandList10* CommandList) {
    if (CommandList == nullptr) {
        LOG_ERROR(L"\t\tCommandList is null.\n");
        return false;
    }

    if (FAILED(CommandList->Close())) {
        LOG_ERROR(L"\t\tFailed to close command list.\n");
        return false;
    }

    ID3D12CommandList* Lists[1] = {CommandList};
    mD3D12CommandQueue->ExecuteCommandLists(1, Lists);

    NextFenceValue();
    return true;
}

uint64_t CommandQueue::NextFenceValue() {
    std::lock_guard<std::mutex> LockGuard(mFenceValueMutex);

    // Signal the command queue with the next fence value
    mD3D12CommandQueue->Signal(mD3D12Fence.Get(), mNextFenceValue);

    // Return and increment
    return mNextFenceValue++;
}

bool CommandQueue::WaitForFenceValue(uint64_t FenceValueToWait) {
    // Check the fence has already been crossed first
    if (FenceValueToWait > mD3D12Fence->GetCompletedValue()) {
        {  // Synchronized block
            std::lock_guard<std::mutex> LockGuard(mFenceEventMutex);

            if (FAILED(mD3D12Fence->SetEventOnCompletion(FenceValueToWait, mFenceEventHandle))) {
                LOG_ERROR(L"\t\tFailed to set fence event on completion.\n");
                return false;
            }

            if (WAIT_OBJECT_0 != WaitForSingleObject(mFenceEventHandle, INFINITE)) {
                LOG_ERROR(L"\t\tWaitForSingleObject failed.\n");
                return false;
            }
        }
    }
    // LOG_INFO(L"Synchronized at fence value %llu\n",FenceValueToWait);
    return true;
}
