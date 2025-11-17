#pragma once

#include "Includes/MathIncl.h"

// XMVECTOR requires 16byte alignment
ALIGN(16)
class Vector3 {
   public:
    using FXMVECTOR = DirectX::FXMVECTOR;
    using XMVECTOR = DirectX::XMVECTOR;
    using XMFLOAT3 = DirectX::XMFLOAT3;

    // In order to take advantage SIMD we use a load XMLoadFloat3 function
    // to convert XMFLOAT3 -> XMVECTOR
    INLINE Vector3(const XMFLOAT3& v) : mVec(DirectX::XMLoadFloat3(&v)) {}

    INLINE Vector3(float x, float y, float z) : mVec(DirectX::XMVectorSet(x, y, z, 0)) {}

    INLINE Vector3(FXMVECTOR vec) {
        mVec = vec;
    }

    INLINE operator XMVECTOR() const {
        return mVec;
    }

   private:
    XMVECTOR mVec;
};

// XMVECTOR requires 16byte alignment
ALIGN(16)
class Vector4 {
   public:
    using XMVECTOR = DirectX::XMVECTOR;
    using XMFLOAT4 = DirectX::XMFLOAT4;

    // In order to take advantage SIMD we use a load XMLoadFloat4 function
    // to convert XMFLOAT4 -> XMVECTOR
    INLINE Vector4(const XMFLOAT4& v) : mVec(DirectX::XMLoadFloat4(&v)) {}

    INLINE explicit Vector4(DirectX::FXMVECTOR vec) {
        mVec = vec;
    }

    INLINE operator XMVECTOR() const {
        return mVec;
    }

   private:
    XMVECTOR mVec;
};