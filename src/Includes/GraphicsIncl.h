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

#define D3D12_GPU_VIRTUAL_ADDRESS_NULL ((D3D12_GPU_VIRTUAL_ADDRESS)0)
#define D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN ((D3D12_GPU_VIRTUAL_ADDRESS) - 1)
#define D3D12_RESOURCE_STATES_NULL (D3D12_RESOURCE_STATES(-1))
#define D3D12_CPU_DESCRIPTOR_HANDLE_NULL (D3D12_CPU_DESCRIPTOR_HANDLE{})
#define D3D12_GRAPHICS_PIPELINE_STATE_DESC_NULL (D3D12_GRAPHICS_PIPELINE_STATE_DESC{})
#define DXGI_FORMAT_DEFAULT_RTV (DXGI_FORMAT_R8G8B8A8_UNORM)