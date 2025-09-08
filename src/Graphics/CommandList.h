#pragma once

#include "CommandQueue.h"
#include "Includes/GraphicsIncl.h"
#include "Logging/Logging.h"

/**
 * CommandList is a RAII wrapper for any ID3D12GraphicsCommandList version.
 * It ensures that the command list is executed and the command queue is waited on when it goes
 * out of scope.
 */
template <typename TD3D12CommandList,
          typename = std::enable_if_t<
              std::is_base_of<ID3D12GraphicsCommandList, TD3D12CommandList>::value>>
class CommandList {
   public:
    CommandList() = default;

    CommandList(CommandQueue* CommandQueue, TD3D12CommandList* CommandList)
        : mCommandQueue(CommandQueue), mCommandList(CommandList) {}

    ~CommandList() {
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
    CommandList(const CommandList& copy) = delete;
    CommandList& operator=(const CommandList& copy) = delete;

    // Move constructor
    CommandList(CommandList&& other) noexcept
        : mCommandQueue(other.mCommandQueue), mCommandList(other.mCommandList) {
        other.mCommandQueue = nullptr;
        other.mCommandList = nullptr;
    }

    // Move assignment operator
    CommandList& operator=(CommandList&& other) noexcept {
        if (this != &other) {
            // The command list doesn't own these resources, so just move the pointers
            mCommandQueue = other.mCommandQueue;
            mCommandList = other.mCommandList;
            other.mCommandQueue = nullptr;
            other.mCommandList = nullptr;
        }
        return *this;
    }

    /**
     * Allows CommandList to be used as if it were a pointer to the command list type.
     */
    TD3D12CommandList* operator->() const {
        return mCommandList;
    }

   private:
    CommandQueue* mCommandQueue{nullptr};
    TD3D12CommandList* mCommandList{nullptr};
};
