// Helper header to configure and include necessary DirectX headers.
#pragma once

#include <d3d12.h>
#include <d3dx12.h>
#include <dxgi1_6.h>

// Include DX debug headers conditionally
#ifdef _DEBUG
#include <d3d12sdklayers.h>
#include <dxgidebug.h>
#endif