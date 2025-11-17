# TODO

## Resource State Tracking for Concurrent Command Lists

### Problem Statement

The current `CommandList10::TransitionResource()` method has a critical race condition when multiple command lists access the same resource concurrently. The method reads the current state with `GetCurrentState()`, then updates it with `SetCurrentState()` inside the conditional. If another command list transitions the same resource concurrently, this results in incorrect state tracking and potentially invalid resource barriers.

**Current Issue:**
- Resource state is stored per-resource (`Resource::mState`)
- Multiple command lists can read the same state simultaneously
- State updates are not atomic, leading to race conditions
- Resource barriers may be recorded with incorrect `StateBefore` values

### Solution: D3D12 Enhanced Barriers API

The D3D12 Enhanced Barriers API (introduced in DirectX 12 Agility SDK) provides automatic resource state tracking, eliminating the need for manual state management and resolving the race condition entirely. Instead of manually tracking `StateBefore` and `StateAfter`, the runtime automatically tracks resource layouts.

**Implementation Tasks:**
- Migrate from `ID3D12GraphicsCommandList::ResourceBarrier()` to `ID3D12GraphicsCommandList9::Barrier()`
- Replace `D3D12_RESOURCE_STATES` with `D3D12_BARRIER_LAYOUT` for resource state representation
- Update `TransitionResource()` method to use Enhanced Barriers API
- Remove manual state tracking (`Resource::mState`, `GetCurrentState()`, `SetCurrentState()`)
- Update command list interface to use `ID3D12GraphicsCommandList9` instead of `ID3D12GraphicsCommandList10`

**Key Benefits:**
- **Automatic State Tracking**: Runtime handles resource state transitions automatically
- **No Race Conditions**: Eliminates the need for manual synchronization or centralized state tracking
- **Simplified Code**: No need to track `StateBefore` - runtime knows current state
- **Better Performance**: Reduced overhead from manual state management
- **Explicit Synchronization**: Enhanced Barriers provide more granular control over synchronization scopes

**Design Considerations:**
- Requires DirectX 12 Agility SDK (Windows 11 22H2+ or Windows 10 with Agility SDK)
- Command list must be created with `ID3D12GraphicsCommandList9` interface
- Use `D3D12_BARRIER_LAYOUT` enum instead of `D3D12_RESOURCE_STATES`
- Enhanced Barriers use `D3D12_BARRIER_GROUP` structures for batched barrier operations
- Supports split barriers for better GPU parallelism

**References:**
- **[D3D12 Enhanced Barriers Specification](https://microsoft.github.io/DirectX-Specs/d3d/D3D12EnhancedBarriers.html)** - Official Microsoft specification document for Enhanced Barriers API, including design details, API reference, and migration guide from legacy Resource Barriers
- **[Microsoft DirectX-Graphics-Samples - D3D12EnhancedBarriers](https://github.com/microsoft/DirectX-Graphics-Samples/tree/master/Samples/Desktop/D3D12EnhancedBarriers)** - Official Microsoft sample demonstrating Enhanced Barriers API usage
- **[microsoft/D3D12TranslationLayer](https://github.com/microsoft/D3D12TranslationLayer)** - Production example of resource state management patterns
- **[microsoft/DirectXTK12](https://github.com/microsoft/DirectXTK12)** - Helper patterns for resource barrier management
- **[NVIDIA-RTX/NRI](https://github.com/NVIDIA-RTX/NRI)** - Low-level render interface with D3D12 support and advanced features

### Example Application: Multi-Pass Rendering (Parallel Recording)

A practical example that showcases concurrent resource access and how Enhanced Barriers API eliminates race conditions:

**Scenario: Parallel Command List Recording for Multi-Pass Rendering**
- **Thread 1**: Records command list for opaque pass
  - Renders opaque objects to render target texture (`D3D12_BARRIER_LAYOUT_RENDER_TARGET`)
  - Accesses shared vertex/index buffers with `D3D12_BARRIER_ACCESS_VERTEX_BUFFER` (buffers have no layout, only access)
  - Transitions render target texture from `D3D12_BARRIER_LAYOUT_RENDER_TARGET` to `D3D12_BARRIER_LAYOUT_SHADER_RESOURCE` when done
- **Thread 2**: Records command list for transparent pass (concurrently)
  - Reads from the same render target texture as `D3D12_BARRIER_LAYOUT_SHADER_RESOURCE` (for depth testing)
  - Renders transparent objects to render target (`D3D12_BARRIER_LAYOUT_RENDER_TARGET`)
  - Accesses same shared vertex/index buffers with `D3D12_BARRIER_ACCESS_VERTEX_BUFFER`
- **Thread 3**: Records command list for post-process pass (concurrently)
  - Reads from render target texture as `D3D12_BARRIER_LAYOUT_SHADER_RESOURCE` (for blur, color grading, etc.)
  - Renders final result to swap chain back buffer (`D3D12_BARRIER_LAYOUT_RENDER_TARGET`)
  - Accesses same shared vertex/index buffers with `D3D12_BARRIER_ACCESS_VERTEX_BUFFER`

**Why This Matters:**
- Multiple threads **record** command lists simultaneously for different render passes (CPU-side parallelization)
- Command lists **execute sequentially** on the GPU (opaque → transparent → post-process)
- All threads access shared resources (render target textures, vertex/index buffers, swap chain back buffer)
- With traditional `ResourceBarrier()` API, manual state tracking leads to race conditions when threads read/write state concurrently during recording
- Race conditions cause incorrect resource barriers, GPU validation errors, or rendering corruption
- **Enhanced Barriers API solves this**: Runtime automatically tracks resource layout internally. Each thread can safely call `Barrier()` specifying only the target layout (no `StateBefore` needed), eliminating race conditions during recording. The runtime resolves the actual state transitions when command lists execute sequentially on the GPU.

**Key Distinction:**
- **Parallel Recording** (CPU): Multiple threads record command lists simultaneously - this is where race conditions occur with manual state tracking
- **Sequential Execution** (GPU): Command lists execute in order - Enhanced Barriers runtime resolves correct state transitions based on recorded barriers and execution order

**Alternative Scenarios:**
- **Render-to-Texture with Post-Processing**: Similar to multi-pass rendering, but with a dedicated offscreen render target that's read by a post-processing pass
- **Async Compute + Graphics**: Compute shader and graphics command lists both access shared UAV textures/buffers concurrently on different queues (truly parallel execution)
- **Multi-Queue Rendering**: Graphics queue and compute queue both transition shared resources concurrently (truly parallel execution)

**GitHub Examples for Multi-Pass Rendering with Barriers:**
- **[microsoft/DirectX-Graphics-Samples - D3D12EnhancedBarriers](https://github.com/microsoft/DirectX-Graphics-Samples/tree/master/Samples/Desktop/D3D12EnhancedBarriers)** - Official Microsoft sample demonstrating Enhanced Barriers API usage, including multi-threaded command list recording patterns
- **[microsoft/DirectX-Graphics-Samples - Multithreading](https://github.com/microsoft/DirectX-Graphics-Samples/tree/master/Samples/Desktop/D3D12Multithreading)** - Demonstrates parallel command list recording, useful for understanding concurrent resource access patterns
- **[Delt06/dx12-renderer](https://github.com/Delt06/dx12-renderer)** - DirectX 12 renderer with deferred and forward rendering paths, showcases multi-pass rendering with multiple light passes and post-processing effects
- **[MethanePowered/MethaneKit](https://github.com/MethanePowered/MethaneKit)** - C++20 cross-platform framework with "Shadow Cube" tutorial demonstrating multi-pass rendering with render passes
- **[microsoft/D3D12TranslationLayer](https://github.com/microsoft/D3D12TranslationLayer)** - Production-quality resource state management implementation, includes Enhanced Barriers support
- **[microsoft/DirectXTK12 - Resource Barriers Guide](https://github.com/microsoft/DirectXTK12/wiki/Resource-Barriers)** - Comprehensive documentation on resource barrier patterns, applicable to multi-pass rendering scenarios

**Implementation Priority:** Medium (becomes critical when implementing multi-threaded command list recording or async compute)


## Scene Structure Implementation

Scene graph system with material batching, dynamic mesh editing, and Blender compatibility. Prioritizes performance through efficient material batching while supporting procedural mesh generation.

### Design Goals

**Pending:**
- Dynamic mesh editing: Support procedural vertex addition/modification at runtime
- Blender compatibility: Scene structure maps to Blender's object/mesh/material hierarchy

**Completed:**
- ~~Material batching: Minimize PSO and RootSignature switches by grouping draw calls by material~~ ✅ DONE

### Core Components

#### SceneNode (`Src/Graphics/Scene/Node.h`)

**Pending:**
- Represents Blender objects with parent-child relationships

**Completed:**
- ~~Lightweight node for hierarchical scene graph~~ ✅ DONE
- ~~Contains: Transform data, parent reference, child nodes, optional MeshInstance reference~~ ✅ DONE
- ~~Capabilities: Compute world transform from hierarchy, manage parent-child relationships, attach/detach renderable MeshInstance~~ ✅ DONE

#### MeshInstance Updates (`Src/Graphics/Mesh/MeshInstance.h`)

**Pending Capabilities:**
- Optional vertex buffers for dynamic mesh editing (CPU upload buffer, GPU buffer)
- Dirty flag to track when vertex data needs GPU update

**Pending Behavior:**
- Draw method supports optional PSO skipping for material batching
- New method to upload vertex data to GPU when dirty
- New method to mark mesh as dirty when vertices change

**Completed:**
- ~~Basic scene graph support: Works with `Node` for constant buffer updates~~ ✅ DONE
- ~~Existing functionality preserved: Mesh/material references, constant buffer updates~~ ✅ DONE

#### Renderer Updates (`Src/Graphics/Renderer.h`)

**Pending:**
- `UpdateMeshes` helper for dirty mesh updates

**Completed:**
- ~~Extend Renderer to work with root `Node` instead of single `MeshInstance`~~ ✅ DONE
- ~~Renderer holds reference to root `Node` (replaces single `MeshInstance` reference)~~ ✅ DONE
- ~~Scene traversal: Uses `Node::TraverseDepthFirst` to traverse scene graph~~ ✅ DONE
- ~~Rendering queue building: Collects `MeshInstance`s into material-sorted queues (inline in `Update` method)~~ ✅ DONE
- ~~Draw performs material-batched rendering for opaque objects~~ ✅ DONE

### Rendering Pipeline

#### Material Batching

**Completed:**
- ~~BuildRenderQueue traverses scene graph, collects `MeshInstance`s into material buckets (opaque)~~ ✅ DONE
- ~~Renderer sets PSO once per material for opaque objects, draws all meshes in that batch~~ ✅ DONE

### Dynamic Mesh Editing

**Update Mechanism:**
- CPU-side editing modifies vertex data in upload buffer
- Mark mesh as dirty when vertices change
- `UpdateMeshes` helper uploads dirty meshes to GPU buffers
- Rendering queue building runs each frame, always reflects current state

**Completed:**
- ~~Existing constant buffer updates remain unchanged~~ ✅ DONE

### Integration

**Pending:**
- Helper functions for mesh updates (`UpdateMeshes`)

**Completed:**
- ~~New `Node` class works with `MeshInstance`~~ ✅ DONE
- ~~`MeshInstance` supports scene graph use~~ ✅ DONE
- ~~Update `Renderer` to work with root `Node` instead of single `MeshInstance`~~ ✅ DONE
- ~~Scene traversal and rendering queue building~~ ✅ DONE

### File Structure

```
Src/Graphics/
├── Scene/
│   └── Node.h/cpp ✅ DONE
└── Mesh/
    ├── Mesh.h (existing)
    └── MeshInstance.h/cpp (basic scene graph support done, dynamic editing pending)
```

### Implementation Phases

**Pending:**
4. **MeshInstance Extension**: Add optional vertex buffers, dirty flag, modify `Draw` to support PSO skipping, add `UpdateMesh` method
5. **Dynamic Mesh Support**: Complete `UpdateMesh` implementation, `UpdateMeshes` helper, procedural vertex testing
6. **Blender Integration (Future)**: Scene importer, object/mesh/material mapping

**Completed:**
1. ~~**Core Structure**: `Node` (references `MeshInstance`), world transform computation~~ ✅ DONE
2. ~~**Renderer Updates**: Update `Renderer` to work with root `Node`, scene traversal, material batching~~ ✅ DONE
3. ~~**Opaque Pass Rendering**: Material-batched rendering for opaque objects~~ ✅ DONE

### Performance Notes

- Per-frame batch building: acceptable cost for dynamic meshes (<1ms for thousands of objects)
- Material hash map: O(1) insertion, no sorting for opaque objects
- World transform caching: cache matrices, recompute only when local transform changes
- Dirty flags: only update meshes that changed


---

# DONE

## Scene Structure Implementation - Core Components

### Node (SceneNode) Implementation ✅

**File**: `Src/Graphics/Scene/Node.h` and `Node.cpp`

- Lightweight node for hierarchical scene graph
- Contains: Transform data (`Matrix4`), parent reference, child nodes, optional `MeshInstance` reference
- Capabilities: Compute world transform from hierarchy, manage parent-child relationships
- Static depth-first traversal method (`TraverseDepthFirst`)
- Factory method `Create` for creating nodes with `MaterialId` and `MeshInstance`
- Move semantics properly implemented with parent-child relationship management

### Renderer Integration ✅

**File**: `Src/Graphics/Renderer.h` and `Renderer.cpp`

- `Renderer` works with root `Node` instead of single `MeshInstance`
- Uses `Node::TraverseDepthFirst` to traverse scene graph each frame
- Builds rendering queue inline in `Update` method, grouping by `MaterialId`
- Uses `RenderingKey` structure for efficient material-based sorting
- Material batching: Sets PSO once per material, draws all meshes in batch
- Opaque pass rendering implemented with material batching

### Material Batching ✅

- Groups draw calls by `MaterialId` to minimize PSO switches
- Uses `std::set<RenderingKey>` for automatic sorting by material
- PSO is set once per material, then all meshes with that material are drawn
- Implemented in `Renderer::Draw` method

### Examples Integration ✅

- **MaterialBatching**: Demonstrates material batching with multiple triangles using different materials
- **PainTriangle**: Uses `Node`-based scene graph with single triangle
- **WorldSpace**: Demonstrates hierarchical transforms with parent-child relationships
- All examples use `Device::CreateMeshNode` factory method
- All examples use `Renderer::SetScene` to set root `Node`

### Device Integration ✅

**File**: `Src/Graphics/Device.h` and `Device.cpp`

- `CreateMeshNode` factory method creates `Node` with `MaterialId` and `MeshInstance`
- Properly manages resource creation and ownership
