#pragma once
#include <memory>

#include "Graphics/CommandList10.h"
#include "Graphics/Material/Material.h"
#include "Graphics/Resource/UploadBuffer.h"
#include "Mesh.h"

class MeshInstance {
   public:
    MeshInstance(std::shared_ptr<Mesh>&& Mesh,
                 std::shared_ptr<Material> Material,
                 std::unique_ptr<UploadBuffer>&& CpuBuffer,
                 std::unique_ptr<ByteBuffer>&& GpuBuffer)
        : mMeshConstCpuBuffer{std::move(CpuBuffer)},
          mMeshConstGpuBuffer{std::move(GpuBuffer)},
          mMaterial(std::move(Material)),
          mMesh(std::move(Mesh)) {}

    // Prohibit copying
    MeshInstance(const MeshInstance&) = delete;
    MeshInstance& operator=(const MeshInstance&) = delete;

    // Allow moving
    MeshInstance(MeshInstance&& other) noexcept
        : mMeshConstCpuBuffer(std::move(other.mMeshConstCpuBuffer)),
          mMeshConstGpuBuffer(std::move(other.mMeshConstGpuBuffer)),
          mMaterial(std::move(other.mMaterial)),
          mMesh(std::move(other.mMesh)) {}

    MeshInstance& operator=(MeshInstance&& other) noexcept {
        if (this != &other) {
            mMeshConstCpuBuffer = std::move(other.mMeshConstCpuBuffer);
            mMeshConstGpuBuffer = std::move(other.mMeshConstGpuBuffer);
            mMaterial = std::move(other.mMaterial);
            mMesh = std::move(other.mMesh);
        }
        return *this;
    }

    bool Update(CommandList10& Cmdl, float DeltaTime);
    bool Draw(CommandList10& Cmdl) const;

    std::shared_ptr<Material> GetMaterial() const {
        return mMaterial;
    }

    std::shared_ptr<Mesh> GetMesh() const {
        return mMesh;
    }

   private:
    std::unique_ptr<UploadBuffer> mMeshConstCpuBuffer;
    std::unique_ptr<ByteBuffer> mMeshConstGpuBuffer;
    std::shared_ptr<Material> mMaterial;
    std::shared_ptr<Mesh> mMesh;
};
