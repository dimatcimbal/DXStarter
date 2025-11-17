#pragma once

#include "Includes/ComIncl.h"
#include "Includes/GraphicsIncl.h"
#include "Logging/Logging.h"

class CommandAllocator {
    // Alias for Microsoft::WRL::ComPtr
    template <typename T>
    using ComPtr = Microsoft::WRL::ComPtr<T>;

   public:
    CommandAllocator(D3D12_COMMAND_LIST_TYPE Type,
                     ComPtr<ID3D12CommandAllocator>&& pD3D12CommandAllocator,
                     ComPtr<ID3D12GraphicsCommandList10>&& pGraphicsCommandList)
        : mType{Type},
          mD3D12CommandAllocator{std::move(pD3D12CommandAllocator)},
          mD3D12GraphicsCommandList{std::move(pGraphicsCommandList)} {}

    ~CommandAllocator() {
        LOG_INFO(L"Freeing CommandAllocator of type %d\n", mType);
    }

    // Prohibit copying
    CommandAllocator(const CommandAllocator& copy) = delete;
    CommandAllocator& operator=(const CommandAllocator& copy) = delete;

    // Get a fresh command list
    bool GetID3D12CommandList(ID3D12GraphicsCommandList10*& OutCommandList) const;

   private:
    D3D12_COMMAND_LIST_TYPE mType;
    ComPtr<ID3D12CommandAllocator> mD3D12CommandAllocator;
    ComPtr<ID3D12GraphicsCommandList10> mD3D12GraphicsCommandList;
};
