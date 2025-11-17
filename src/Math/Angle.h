#pragma once

#include "DirectXMath.h"

class Degrees {
   public:
    Degrees(float degrees) : mDeg(degrees) {}

    // Performs implicit conversion from degrees to radians
    operator float() const {
        return DirectX::XMConvertToRadians(mDeg);
    }

   private:
    float mDeg;
};
