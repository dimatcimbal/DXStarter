#pragma once

#include <memory>

// Forward declarations
class Device;
class Bytes;
class Material;

/**
 * Builder class for creating Material instances.
 * Handles shader bytecode configuration and pipeline state creation.
 */
class MaterialBuilder {
   public:
    MaterialBuilder() = default;
    ~MaterialBuilder() = default;

    // Prohibit copying
    MaterialBuilder(const MaterialBuilder&) = delete;
    MaterialBuilder& operator=(const MaterialBuilder&) = delete;

    // Allow moving
    MaterialBuilder(MaterialBuilder&& other) noexcept
        : mVertexShaderBytecode(std::move(other.mVertexShaderBytecode)),
          mPixelShaderBytecode(std::move(other.mPixelShaderBytecode)),
          mRootSigBytecode(std::move(other.mRootSigBytecode)) {}

    MaterialBuilder& operator=(MaterialBuilder&& other) noexcept {
        if (this != &other) {
            mVertexShaderBytecode = std::move(other.mVertexShaderBytecode);
            mPixelShaderBytecode = std::move(other.mPixelShaderBytecode);
            mRootSigBytecode = std::move(other.mRootSigBytecode);
        }
        return *this;
    }

    MaterialBuilder& SetVertexShaderBytecode(std::unique_ptr<Bytes>&& Bytecode) {
        mVertexShaderBytecode = std::move(Bytecode);
        return *this;
    }

    MaterialBuilder& SetPixelShaderBytecode(std::unique_ptr<Bytes>&& Bytecode) {
        mPixelShaderBytecode = std::move(Bytecode);
        return *this;
    }

    MaterialBuilder& SetRootSigBytecode(std::unique_ptr<Bytes>&& Bytecode) {
        mRootSigBytecode = std::move(Bytecode);
        return *this;
    }

    bool CreateMaterial(Device& Device, std::shared_ptr<Material>& OutMaterial);

   private:
    std::unique_ptr<Bytes> mVertexShaderBytecode;
    std::unique_ptr<Bytes> mPixelShaderBytecode;
    std::unique_ptr<Bytes> mRootSigBytecode;
};
