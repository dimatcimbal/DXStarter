#include "MeshInstance.h"

void MeshInstance::WriteToUploadBuffer(const Matrix4& WorldTransform) const {
    BufferRange bufferRange = mUploadConstantBuffer->Map();
    MeshConstantBuffer* cb = static_cast<MeshConstantBuffer*>(bufferRange.GetPtr());
    cb->World = WorldTransform;
}

void MeshInstance::Update(CommandList10& Cmdl, const Matrix4& WorldTransform) const {
    // Write the transform to the CPU constant buffer
    WriteToUploadBuffer(WorldTransform);

    // Update Device constant buffer with the data from the CPU one
    Cmdl.TransitionResource(*mMeshConstantBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
    Cmdl.CopyBufferRegion(*mUploadConstantBuffer, 0, *mMeshConstantBuffer,
                          mUploadConstantBuffer->GetBufferSize());
    Cmdl.TransitionResource(*mMeshConstantBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);
}

void MeshInstance::Draw(const CommandList10& Cmdl) const {
    // Set up vertex buffer view
    Cmdl.SetConstantBuffer(0, *mMeshConstantBuffer);
    Cmdl.SetVertexBuffer(0, *mMesh);
    Cmdl.DrawInstanced(mMesh->GetVertexCount(), 0);
}