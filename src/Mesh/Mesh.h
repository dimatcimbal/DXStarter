#pragma once
#include <utility>

#include "Graphics/Resources/ByteBuffer.h"
#include "Includes/GraphicsIncl.h"

class Mesh {
   public:
    Mesh(uint32_t VertexCount, uint32_t VertexStrideInBytes, ByteBuffer&& VertexBuffer)
        : mVertexCount(VertexCount),
          mVertexStrideInBytes(VertexStrideInBytes),
          mVertexBuffer(std::move(VertexBuffer)) {}

    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    Mesh(Mesh&& other) noexcept
        : mVertexCount(std::exchange(other.mVertexCount, 0)),
          mVertexStrideInBytes(std::exchange(other.mVertexStrideInBytes, 0)),
          mVertexBuffer(std::move(other.mVertexBuffer)) {}

    Mesh& operator=(Mesh&& other) noexcept {
        if (this != &other) {
            mVertexCount = std::exchange(other.mVertexCount, 0),
            mVertexStrideInBytes = std::exchange(other.mVertexStrideInBytes, 0);
            mVertexBuffer = std::move(other.mVertexBuffer);
        }
        return *this;
    }

    D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView() {
        D3D12_VERTEX_BUFFER_VIEW vbv;
        vbv.BufferLocation = GetVertexBuffer();
        vbv.SizeInBytes = GetVertexBufferSize();
        vbv.StrideInBytes = GetStrideInBytes();
        return vbv;
    }

    D3D12_GPU_VIRTUAL_ADDRESS GetVertexBuffer() const {
        return mVertexBuffer.GetGPUVirtualAddress();
    }

    size_t GetVertexBufferSize() const {
        return mVertexBuffer.GetBufferSize();
    }

    uint32_t GetStrideInBytes() const {
        return mVertexStrideInBytes;
    }

    uint32_t GetVertexCount() const {
        return mVertexCount;
    }

   private:
    ByteBuffer mVertexBuffer;
    uint32_t mVertexStrideInBytes;
    uint32_t mVertexCount;
};
