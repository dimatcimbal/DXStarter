#pragma once

#include "Angle.h"
#include "Includes/MathIncl.h"
#include "Vector.h"

// XMMATRIX requires 16byte alignment
ALIGN(16)
class Matrix4 {
   public:
    using XMMATRIX = DirectX::XMMATRIX;
    using XMVECTOR = DirectX::XMVECTOR;
    using XMFLOAT4X4 = DirectX::XMFLOAT4X4;

    INLINE Matrix4() : mMat(DirectX::XMMatrixIdentity()) {}

    INLINE Matrix4(const XMMATRIX& mat) : mMat(mat) {}

    // In order to take advantage SIMD we use a load function XMLoadFloat4x4
    // to convert XMFLOAT4X4 -> XMMATRIX
    INLINE Matrix4(const float* m)
        : mMat(DirectX::XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(m))) {}

    INLINE operator XMMATRIX() const {
        return mMat;
    }

    INLINE Vector4 operator*(Vector3 vec) const {
        return Vector4(DirectX::XMVector3Transform(vec, mMat));
    }
    INLINE Vector4 operator*(Vector4 vec) const {
        return Vector4(DirectX::XMVector4Transform(vec, mMat));
    }
    INLINE Matrix4 operator*(const Matrix4& mat) const {
        return Matrix4(DirectX::XMMatrixMultiply(mat, mMat));
    }

    // Setters
    INLINE Matrix4& RotateX(Degrees degrees) {
        mMat = DirectX::XMMatrixMultiply(mMat,
                                         DirectX::XMMatrixRotationX(static_cast<float>(degrees)));
        return *this;
    }

    INLINE Matrix4& RotateY(Degrees degrees) {
        mMat = DirectX::XMMatrixMultiply(mMat,
                                         DirectX::XMMatrixRotationY(static_cast<float>(degrees)));
        return *this;
    }

    INLINE Matrix4& RotateZ(Degrees degrees) {
        mMat = DirectX::XMMatrixMultiply(mMat,
                                         DirectX::XMMatrixRotationZ(static_cast<float>(degrees)));
        return *this;
    }

    INLINE Matrix4& Scale(float scale) {
        mMat = DirectX::XMMatrixMultiply(mMat, DirectX::XMMatrixScaling(scale, scale, scale));
        return *this;
    }

    INLINE Matrix4& Translate(Vector3 vec) {
        mMat = DirectX::XMMatrixMultiply(mMat, DirectX::XMMatrixTranslationFromVector(vec));
        return *this;
    }

   private:
    XMMATRIX mMat;
};
