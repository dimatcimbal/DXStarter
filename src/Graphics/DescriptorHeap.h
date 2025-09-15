#pragma once

#include "Includes/ComIncl.h"
#include "Includes/GraphicsIncl.h"
#include "Logging/Logging.h"

class DescriptorHeap {
   public:
    DescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE Type,
                   uint32_t Size,
                   uint32_t Count,
                   Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>&& DescriptorHeap)
        : mType{Type},
          mSize{Size},
          mFreeDescriptorCount{Count},
          mFirstHandle{DescriptorHeap->GetCPUDescriptorHandleForHeapStart()},
          mCurrentHandle{mFirstHandle},
          mD3D12Heap{std::move(DescriptorHeap)} {}

    // Prohibit copying
    DescriptorHeap(const DescriptorHeap& other) = delete;
    DescriptorHeap& operator=(const DescriptorHeap& other) = delete;

    // Instance members
    bool AllocateHandles(uint32_t Count, D3D12_CPU_DESCRIPTOR_HANDLE& OutHandle) {
        // Overflow check
        if (Count > mFreeDescriptorCount) {
            LOG_ERROR(
                L"\t\tNot enough free descriptors in the heap of type %d. Requested: %u, "
                L"Available: %u\n",
                mType, Count, mFreeDescriptorCount);
            return false;
        }

        // Return the current handle
        OutHandle = mCurrentHandle;

        // Incrementing to the next handle
        mCurrentHandle.ptr += Count * mSize;
        mFreeDescriptorCount -= Count;
        return true;
    }

   private:
    uint32_t mSize;
    uint32_t mFreeDescriptorCount;

    D3D12_DESCRIPTOR_HEAP_TYPE mType;
    D3D12_CPU_DESCRIPTOR_HANDLE mFirstHandle;
    D3D12_CPU_DESCRIPTOR_HANDLE mCurrentHandle;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mD3D12Heap;
};