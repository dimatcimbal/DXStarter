#include "MeshInstance.h"

bool MeshInstance::Update(CommandList10& Cmdl, float DeltaTime) {
    // Get the pointer to the buffer range
    BufferRange BufferRange = mMeshConstCpuBuffer->Map();
    // BufferRange.GetPtr() - gives the access to the data
    // TODO:
    // 1. Update the constant buffer data here
    // 2. Copy the data from the CPU buffer to the GPU buffer using Cmdl

    return true;
}

bool MeshInstance::Draw(CommandList10& Cmdl) const {
    // TODO:
    // 1. Set up vertex buffer view
    // 2. Issue draw calls using Cmdl

    Cmdl.SetVertexBuffer(0, mMesh->GetVertexBufferView());
    Cmdl.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    Cmdl.DrawInstanced(mMesh->GetVertexCount(), 0);
    return true;
}