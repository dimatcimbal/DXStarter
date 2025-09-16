#include "CommandAllocator.h"

#include "Logging/Logging.h"

bool CommandAllocator::GetID3D12CommandList(ID3D12GraphicsCommandList10*& OutCommandList) const {
    if (FAILED(mD3D12CommandAllocator->Reset())) {
        LOG_ERROR(L"\t\tFailed to reset ID3D12CommandAllocator\n");
        return false;
    }

    if (FAILED(mD3D12GraphicsCommandList->Reset(mD3D12CommandAllocator.Get(), nullptr))) {
        LOG_ERROR(L"\t\tFailed to reset ID3D12GraphicsCommandList\n");
        return false;
    }

    OutCommandList = mD3D12GraphicsCommandList.Get();
    return true;
}