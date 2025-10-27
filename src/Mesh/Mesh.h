#pragma once
#include <utility>

#include "Graphics/Resources/ByteBuffer.h"
#include "Includes/GraphicsIncl.h"

class Mesh {
   public:
    Mesh(ByteBuffer&& vertexBuffer) : mVertexBuffer(std::move(vertexBuffer)) {}

    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    Mesh(Mesh&& other) : mVertexBuffer(std::move(other.mVertexBuffer)) {}

    Mesh& operator=(Mesh&& other) {
        if (this != &other) {
            mVertexBuffer = std::move(other.mVertexBuffer);
        }
        return *this;
    }

    D3D12_GPU_VIRTUAL_ADDRESS GetVertexData() const {
        return mVertexBuffer.GetGPUVirtualAddress();
    }

    size_t GetVertexBufferSize() const {
        return mVertexBuffer.GetBufferSize();
    }

   private:
    ByteBuffer mVertexBuffer;
};
