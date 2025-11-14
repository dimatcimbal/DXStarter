#pragma once

#include <d3d12.h>

#include <map>
#include <memory>

#include "IO/Bytes.h"

// Forward declarations
class Device;
class Material;

enum class ShaderType { Vertex, Pixel, Hull, Domain, Geometry, RootSignature };

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
        : mPSODesc(other.mPSODesc), mShaderBytecodes(std::move(other.mShaderBytecodes)) {}

    MaterialBuilder& operator=(MaterialBuilder&& other) noexcept {
        if (this != &other) {
            mPSODesc = other.mPSODesc;
            mShaderBytecodes = std::move(other.mShaderBytecodes);
        }
        return *this;
    }

    MaterialBuilder& SetVertexShaderBytecode(std::unique_ptr<Bytes>&& Bytecode) {
        mShaderBytecodes[ShaderType::Vertex] = std::move(Bytecode);
        auto& bytecode = mShaderBytecodes[ShaderType::Vertex];
        mPSODesc.VS.pShaderBytecode = bytecode->GetBuffer();
        mPSODesc.VS.BytecodeLength = bytecode->GetSize();
        return *this;
    }

    MaterialBuilder& SetPixelShaderBytecode(std::unique_ptr<Bytes>&& Bytecode) {
        mShaderBytecodes[ShaderType::Pixel] = std::move(Bytecode);
        auto& bytecode = mShaderBytecodes[ShaderType::Pixel];
        mPSODesc.PS.pShaderBytecode = bytecode->GetBuffer();
        mPSODesc.PS.BytecodeLength = bytecode->GetSize();
        return *this;
    }

    MaterialBuilder& SetHullShaderBytecode(std::unique_ptr<Bytes>&& Bytecode) {
        mShaderBytecodes[ShaderType::Hull] = std::move(Bytecode);
        auto& bytecode = mShaderBytecodes[ShaderType::Hull];
        mPSODesc.HS.pShaderBytecode = bytecode->GetBuffer();
        mPSODesc.HS.BytecodeLength = bytecode->GetSize();
        return *this;
    }

    MaterialBuilder& SetDomainShaderBytecode(std::unique_ptr<Bytes>&& Bytecode) {
        mShaderBytecodes[ShaderType::Domain] = std::move(Bytecode);
        auto& bytecode = mShaderBytecodes[ShaderType::Domain];
        mPSODesc.DS.pShaderBytecode = bytecode->GetBuffer();
        mPSODesc.DS.BytecodeLength = bytecode->GetSize();
        return *this;
    }

    MaterialBuilder& SetGeometryShaderBytecode(std::unique_ptr<Bytes>&& Bytecode) {
        mShaderBytecodes[ShaderType::Geometry] = std::move(Bytecode);
        auto& bytecode = mShaderBytecodes[ShaderType::Geometry];
        mPSODesc.GS.pShaderBytecode = bytecode->GetBuffer();
        mPSODesc.GS.BytecodeLength = bytecode->GetSize();
        return *this;
    }

    MaterialBuilder& SetRootSigBytecode(std::unique_ptr<Bytes>&& Bytecode) {
        mShaderBytecodes[ShaderType::RootSignature] = std::move(Bytecode);
        return *this;
    }

    bool CreateMaterial(Device& Device, std::shared_ptr<Material>& OutMaterial);

   private:
    D3D12_GRAPHICS_PIPELINE_STATE_DESC mPSODesc;
    std::map<ShaderType, std::unique_ptr<Bytes>> mShaderBytecodes;
};
