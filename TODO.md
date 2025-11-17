# TODO

## Scene Structure Implementation

Scene graph system with material batching, dynamic mesh editing, and Blender compatibility. Prioritizes performance through efficient material batching while supporting procedural mesh generation.

### Design Goals
- ~~Material batching: Minimize PSO and RootSignature switches by grouping draw calls by material~~ ✅ DONE
- Dynamic mesh editing: Support procedural vertex addition/modification at runtime
- Blender compatibility: Scene structure maps to Blender's object/mesh/material hierarchy
- Two-pass rendering: Separate opaque (material-batched) and transparent (depth-sorted) objects (opaque pass done, transparent pass pending)

### Core Components

**SceneNode** (`Src/Graphics/Scene/Node.h`) ✅ DONE
- ~~Lightweight node for hierarchical scene graph~~ ✅ DONE
- ~~Contains: Transform data, parent reference, child nodes, optional MeshInstance reference~~ ✅ DONE
- ~~Capabilities: Compute world transform from hierarchy, manage parent-child relationships, attach/detach renderable MeshInstance~~ ✅ DONE
- Represents Blender objects with parent-child relationships

**MeshInstance Updates** (`Src/Graphics/Mesh/MeshInstance.h`)
- ~~Basic scene graph support: Works with Node for constant buffer updates~~ ✅ DONE
- **Pending capabilities:**
  - Optional vertex buffers for dynamic mesh editing (CPU upload buffer, GPU buffer)
  - Dirty flag to track when vertex data needs GPU update
  - Depth value for sorting (distance from camera)
  - Transparency classification (opaque vs transparent)
- **Pending behavior:**
  - Draw method supports optional PSO skipping for material batching
  - New method to upload vertex data to GPU when dirty
  - New method to mark mesh as dirty when vertices change
- **Existing functionality preserved:** Mesh/material references, constant buffer updates ✅ DONE

**Renderer Updates** (`Src/Graphics/Renderer.h`) ✅ DONE
- ~~Extend Renderer to work with root Node instead of single MeshInstance~~ ✅ DONE
- ~~Renderer holds reference to root Node (replaces single MeshInstance reference)~~ ✅ DONE
- ~~Scene traversal: Uses Node::TraverseDepthFirst to traverse scene graph~~ ✅ DONE
- ~~Rendering queue building: Collects MeshInstances into material-sorted queues (inline in Update method)~~ ✅ DONE
- ~~Draw performs material-batched rendering for opaque objects~~ ✅ DONE
- **Pending:**
  - Separate BuildRenderQueue helper function (functionality currently inline)
  - UpdateMeshes helper for dirty mesh updates
  - Two-pass rendering (transparent pass with depth sorting)

**Transform** (`Src/Graphics/Scene/Transform.h`)
- Transform data structure for position, rotation (quaternion), scale
- Capabilities: Compute local transform matrix, compose with parent transform for world matrix
- **Note:** Currently Node uses Matrix4 directly for transforms. Separate Transform class is optional.

### Rendering Pipeline

**Two-Pass Strategy:**
- ~~Pass 1 (Opaque): Collect MeshInstances, group by Material, set PSO once per material, draw all meshes in batch. Benefits: minimal PSO switches, early Z rejection.~~ ✅ DONE
- Pass 2 (Transparent): Collect MeshInstances, sort by depth (far-to-near), render in sorted order for correct alpha blending.

**Material Batching:** ✅ DONE
- ~~BuildRenderQueue traverses scene graph, collects MeshInstances into material buckets (opaque)~~ ✅ DONE
- ~~Renderer sets PSO once per material for opaque objects, draws all meshes in that batch~~ ✅ DONE
- Pending: Transparent objects depth-sorted rendering

### Dynamic Mesh Editing

**Update Mechanism:**
- CPU-side editing modifies vertex data in upload buffer
- Mark mesh as dirty when vertices change
- UpdateMeshes helper uploads dirty meshes to GPU buffers
- BuildRenderQueue runs each frame, always reflects current state
- ~~Existing constant buffer updates remain unchanged~~ ✅ DONE

### Integration

- ~~New `Node` class works with `MeshInstance`~~ ✅ DONE
- ~~`MeshInstance` supports scene graph use~~ ✅ DONE
- ~~Update `Renderer` to work with root `Node` instead of single `MeshInstance`~~ ✅ DONE
- ~~Scene traversal and rendering queue building~~ ✅ DONE
- Pending: Helper functions for mesh updates (UpdateMeshes)

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
1. ~~**Core Structure**: Node (references MeshInstance), world transform computation~~ ✅ DONE
2. ~~**Renderer Updates**: Update Renderer to work with root Node, scene traversal, material batching~~ ✅ DONE
3. ~~**Opaque Pass Rendering**: Material-batched rendering for opaque objects~~ ✅ DONE
4. **MeshInstance Extension**: Add optional vertex buffers, dirty flag, depth/transparency flags, modify Draw to support PSO skipping, add UpdateMesh method
5. **Transparent Pass Rendering**: Two-pass rendering with depth-sorted transparent objects
6. **Dynamic Mesh Support**: Complete UpdateMesh implementation, UpdateMeshes helper, procedural vertex testing
7. **Blender Integration (Future)**: Scene importer, object/mesh/material mapping

### Performance Notes
- Per-frame batch building: acceptable cost for dynamic meshes (<1ms for thousands of objects)
- Material hash map: O(1) insertion, no sorting for opaque objects
- Depth sorting: only for transparent objects (usually fewer)
- World transform caching: cache matrices, recompute only when local transform changes
- Dirty flags: only update meshes that changed



# DONE

## Scene Structure Implementation - Core Components

### Node (SceneNode) Implementation ✅
- **File**: `Src/Graphics/Scene/Node.h` and `Node.cpp`
- Lightweight node for hierarchical scene graph
- Contains: Transform data (Matrix4), parent reference, child nodes, optional MeshInstance reference
- Capabilities: Compute world transform from hierarchy, manage parent-child relationships
- Static depth-first traversal method (`TraverseDepthFirst`)
- Factory method `Create` for creating nodes with MaterialId and MeshInstance
- Move semantics properly implemented with parent-child relationship management

### Renderer Integration ✅
- **File**: `Src/Graphics/Renderer.h` and `Renderer.cpp`
- Renderer works with root Node instead of single MeshInstance
- Uses `Node::TraverseDepthFirst` to traverse scene graph each frame
- Builds rendering queue inline in `Update` method, grouping by MaterialId
- Uses `RenderingKey` structure for efficient material-based sorting
- Material batching: Sets PSO once per material, draws all meshes in batch
- Opaque pass rendering implemented with material batching

### Material Batching ✅
- Groups draw calls by MaterialId to minimize PSO switches
- Uses `std::set<RenderingKey>` for automatic sorting by material
- PSO is set once per material, then all meshes with that material are drawn
- Implemented in `Renderer::Draw` method

### Examples Integration ✅
- **MeshBatching**: Demonstrates material batching with multiple triangles using different materials
- **PainTriangle**: Uses Node-based scene graph with single triangle
- **RotatedTriangle**: Demonstrates hierarchical transforms with parent-child relationships
- All examples use `Device::CreateMeshNode` factory method
- All examples use `Renderer::SetScene` to set root Node

### Device Integration ✅
- **File**: `Src/Graphics/Device.h` and `Device.cpp`
- `CreateMeshNode` factory method creates Node with MaterialId and MeshInstance
- Properly manages resource creation and ownership

