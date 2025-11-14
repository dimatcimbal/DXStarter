#pragma once

#include <utility>

#include "Includes/ComIncl.h"
#include "Includes/GraphicsIncl.h"

class PipelineState {
   public:
    PipelineState(Microsoft::WRL::ComPtr<ID3D12PipelineState>&& PipelineState)
        : mPipelineState(std::move(PipelineState)) {}

    // Prohibit copying
    PipelineState(const PipelineState&) = delete;
    PipelineState& operator=(const PipelineState&) = delete;

    // Allow moving
    PipelineState(PipelineState&& other) noexcept
        : mPipelineState(std::exchange(other.mPipelineState, nullptr)) {}

    PipelineState& operator=(PipelineState&& other) noexcept {
        if (this != &other) {
            mPipelineState = std::exchange(other.mPipelineState, nullptr);
        }
        return *this;
    }

    ID3D12PipelineState* GetD3DPipelineState() const {
        return mPipelineState ? mPipelineState.Get() : nullptr;
    }

   private:
    Microsoft::WRL::ComPtr<ID3D12PipelineState> mPipelineState;
};
