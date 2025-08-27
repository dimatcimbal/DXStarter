#include "CommandQueue.h"

bool CommandQueue::ExecuteCommandList(ID3D12GraphicsCommandList10* CommandList) {
    if FAILED (CommandList->Close()) {
        LOG_ERROR(L"Failed to close command list.\n");
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
    std::lock_guard<std::mutex> LockGuard(mFenceEventMutex);

    // Wait for the signal to complete
    if (FenceValueToWait > mD3D12Fence->GetCompletedValue()) {
        if FAILED (mD3D12Fence->SetEventOnCompletion(FenceValueToWait, mFenceEventHandle)) {
            LOG_ERROR(L"Failed to set fence event on completion.\n");
            return false;
        }

        WaitForSingleObject(mFenceEventHandle, INFINITE);
    }
    return true;
}
