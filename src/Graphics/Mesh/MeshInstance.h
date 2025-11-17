#pragma once
#include <memory>

#include "Graphics/CommandList10.h"
#include "Graphics/Resource/UploadBuffer.h"
#include "Math/Matrix.h"
#include "Mesh.h"

struct MeshConstantBuffer {
    Matrix4 World;
};

class MeshInstance {
   public:
    MeshInstance(Mesh& Mesh,
                 std::unique_ptr<UploadBuffer>&& UploadBuffer,
                 std::unique_ptr<DeviceBuffer>&& DeviceBuffer)
        : mUploadConstantBuffer{std::move(UploadBuffer)},
          mMeshConstantBuffer{std::move(DeviceBuffer)},
          mMesh(&Mesh) {}

    // Prohibit copying
    MeshInstance(const MeshInstance&) = delete;
    MeshInstance& operator=(const MeshInstance&) = delete;

    // Allow moving
    MeshInstance(MeshInstance&& other) noexcept
        : mUploadConstantBuffer(std::exchange(other.mUploadConstantBuffer, nullptr)),
          mMeshConstantBuffer(std::exchange(other.mMeshConstantBuffer, nullptr)),
          mMesh(std::exchange(other.mMesh, nullptr)) {}

    MeshInstance& operator=(MeshInstance&& other) noexcept {
        if (this != &other) {
            mUploadConstantBuffer = std::exchange(other.mUploadConstantBuffer, nullptr);
            mMeshConstantBuffer = std::exchange(other.mMeshConstantBuffer, nullptr);
            mMesh = std::exchange(other.mMesh, nullptr);
        }
        return *this;
    }

    void Update(CommandList10& Cmdl, Matrix4& WorldTransform);
    void Draw(const CommandList10& Cmdl) const;

    Mesh* GetMesh() const {
        return mMesh;
    }

    D3D12_GPU_VIRTUAL_ADDRESS GetConstantBuffer() const {
        return mMeshConstantBuffer->GetDeviceVirtualAddress();
    }

   private:
    std::unique_ptr<UploadBuffer> mUploadConstantBuffer;
    std::unique_ptr<DeviceBuffer> mMeshConstantBuffer;
    Mesh* mMesh;
};
