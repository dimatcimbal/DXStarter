#pragma once

#include <DirectXMath.h>

// We want to inline small hot functions like matrix/vector operations
#define INLINE __forceinline

// MSVC specific
#define ALIGN(arg) __declspec(align(arg))