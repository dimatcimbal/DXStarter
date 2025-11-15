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
    if (!mMaterial) {
        return false;
    }

    // Set the pipeline state from material
    Cmdl->SetGraphicsRootSignature(mMaterial->GetD3DRootSignature());
    Cmdl->SetPipelineState(mMaterial->GetD3DPipelineState());

    // Set up vertex buffer view
    Cmdl.SetVertexBuffer(0, mMesh->GetVertexBufferView());
    Cmdl.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    Cmdl.DrawInstanced(mMesh->GetVertexCount(), 0);
    return true;
}