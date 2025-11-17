#pragma once

#include <d3d12.h>

#include <memory>

#include "IO/ByteBuffer.h"
#include "RootSignature.h"

// Forward declarations
class Device;
class Material;

/**
 * Builder class for creating Material instances.
 * Handles shader bytecode configuration and pipeline state creation.
 */
class MaterialBuilder {
   public:
    MaterialBuilder() : mPSODesc{} {}
    ~MaterialBuilder() = default;

    // Prohibit copying
    MaterialBuilder(const MaterialBuilder&) = delete;
    MaterialBuilder& operator=(const MaterialBuilder&) = delete;

    // Allow moving
    MaterialBuilder(MaterialBuilder&& other) noexcept
        : mPSODesc(std::exchange(other.mPSODesc, {})) {}

    MaterialBuilder& operator=(MaterialBuilder&& other) noexcept {
        if (this != &other) {
            mPSODesc = std::exchange(other.mPSODesc, {});
        }
        return *this;
    }

    MaterialBuilder& SetVertexShaderBytecode(ByteBuffer& Bytecode) {
        mPSODesc.VS.pShaderBytecode = Bytecode.GetBuffer();
        mPSODesc.VS.BytecodeLength = Bytecode.GetSize();
        return *this;
    }

    MaterialBuilder& SetPixelShaderBytecode(ByteBuffer& Bytecode) {
        mPSODesc.PS.pShaderBytecode = Bytecode.GetBuffer();
        mPSODesc.PS.BytecodeLength = Bytecode.GetSize();
        return *this;
    }

    MaterialBuilder& SetHullShaderBytecode(ByteBuffer& Bytecode) {
        mPSODesc.HS.pShaderBytecode = Bytecode.GetBuffer();
        mPSODesc.HS.BytecodeLength = Bytecode.GetSize();
        return *this;
    }

    MaterialBuilder& SetDomainShaderBytecode(ByteBuffer& Bytecode) {
        mPSODesc.DS.pShaderBytecode = Bytecode.GetBuffer();
        mPSODesc.DS.BytecodeLength = Bytecode.GetSize();
        return *this;
    }

    MaterialBuilder& SetGeometryShaderBytecode(ByteBuffer& Bytecode) {
        mPSODesc.GS.pShaderBytecode = Bytecode.GetBuffer();
        mPSODesc.GS.BytecodeLength = Bytecode.GetSize();
        return *this;
    }

    bool CreateMaterial(Device& Device,
                        RootSignature& RootSignature,
                        std::shared_ptr<Material>& OutMaterial);

   private:
    D3D12_GRAPHICS_PIPELINE_STATE_DESC mPSODesc;
};
