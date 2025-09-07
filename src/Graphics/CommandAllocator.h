#pragma once

#include <mutex>

#include "Includes/ComIncl.h"
#include "Includes/GraphicsIncl.h"
#include "Logging/Logging.h"

class CommandAllocator {
   public:
    CommandAllocator(D3D12_COMMAND_LIST_TYPE Type,
                     Microsoft::WRL::ComPtr<ID3D12CommandAllocator>&& pD3D12CommandAllocator,
                     Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList10>&& pGraphicsCommandList)
        : mType(Type),
          mD3D12CommandAllocator(std::move(pD3D12CommandAllocator)),
          mD3D12GraphicsCommandList(std::move(pGraphicsCommandList)) {}

    ~CommandAllocator() {
        LOG_INFO(L"\t\tFreeing CommandAllocator of type %d\n", mType);
    }

    // Prohibit copying
    CommandAllocator(const CommandAllocator& copy) = delete;
    CommandAllocator& operator=(const CommandAllocator& copy) = delete;

    // Get a fresh command list
    bool GetID3D12CommandList(ID3D12GraphicsCommandList10*& OutCommandList) const;

   private:
    D3D12_COMMAND_LIST_TYPE mType;
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mD3D12CommandAllocator;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList10> mD3D12GraphicsCommandList;
};
