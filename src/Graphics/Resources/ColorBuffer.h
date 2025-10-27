#pragma once

#include "Includes/ComIncl.h"
#include "Includes/GraphicsIncl.h"
#include "Resource.h"

class ColorBuffer : public Resource {
   public:
    ColorBuffer(D3D12_CPU_DESCRIPTOR_HANDLE RTVHandle,
                Microsoft::WRL::ComPtr<ID3D12Resource2>&& pResource)
        : Resource(std::move(pResource)), mRTVHandle{RTVHandle} {}

    // Prohibit copying
    ColorBuffer(const ColorBuffer&) = delete;
    ColorBuffer& operator=(const ColorBuffer&) = delete;

    // Allow moving
    ColorBuffer(ColorBuffer&& other) noexcept
        : Resource(std::move(other)),
          mRTVHandle(std::exchange(other.mRTVHandle, D3D12_CPU_DESCRIPTOR_HANDLE_NULL)) {}
    ColorBuffer& operator=(ColorBuffer&& other) noexcept {
        if (this != &other) {
            Resource::operator=(std::move(other));
            mRTVHandle = std::exchange(other.mRTVHandle, D3D12_CPU_DESCRIPTOR_HANDLE_NULL);
        }
        return *this;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE GetRTV() const {
        return mRTVHandle;
    }

   private:
    D3D12_CPU_DESCRIPTOR_HANDLE mRTVHandle;
};