#include "MeshInstance.h"

void MeshInstance::Update(CommandList10& Cmdl, Matrix4& WorldTransform) {
    // Write the transform to the upload buffer
    BufferRange bufferRange = mUploadConstantBuffer->Map();
    MeshConstantBuffer* cb = static_cast<MeshConstantBuffer*>(bufferRange.GetPtr());
    cb->World = WorldTransform;

    // Update Device constant buffer with the data from the upload one
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