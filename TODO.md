# TODO

## Scene Structure Implementation

Scene graph system with material batching, dynamic mesh editing, and Blender compatibility. Prioritizes performance through efficient material batching while supporting procedural mesh generation.

### Design Goals
- Material batching: Minimize PSO and RootSignature switches by grouping draw calls by material
- Dynamic mesh editing: Support procedural vertex addition/modification at runtime
- Blender compatibility: Scene structure maps to Blender's object/mesh/material hierarchy
- Two-pass rendering: Separate opaque (material-batched) and transparent (depth-sorted) objects

### Core Components

**SceneNode** (`Src/Graphics/Scene/SceneNode.h`)
- Lightweight node for hierarchical scene graph
- Contains: Transform data, parent reference, child nodes, optional MeshInstance reference
- Capabilities: Compute world transform from hierarchy, manage parent-child relationships, attach/detach renderable MeshInstance
- Represents Blender objects with parent-child relationships

**MeshInstance Updates** (`Src/Graphics/Mesh/MeshInstance.h`)
- Extend existing MeshInstance to support scene graph and dynamic mesh editing
- **New capabilities:**
  - Optional vertex buffers for dynamic mesh editing (CPU upload buffer, GPU buffer)
  - Dirty flag to track when vertex data needs GPU update
  - Depth value for sorting (distance from camera)
  - Transparency classification (opaque vs transparent)
- **Modified behavior:**
  - Draw method supports optional PSO skipping for material batching
  - New method to upload vertex data to GPU when dirty
  - New method to mark mesh as dirty when vertices change
- **Existing functionality preserved:** Mesh/material references, constant buffer updates
- Supports both constant buffer updates (existing) and dynamic vertex buffer updates (new)

**Renderer Updates** (`Src/Graphics/Renderer.h`)
- Extend Renderer to work with root SceneNode instead of single MeshInstance
- Renderer holds reference to root SceneNode (replaces single MeshInstance reference)
- Helper functions for scene operations:
  - BuildRenderQueue: Traverses scene graph, collects MeshInstances into material-sorted queues
  - UpdateMeshes: Updates dirty meshes in scene graph
- Draw builds render queue and performs two-pass rendering
- Update handles mesh updates
- No separate Scene class needed - Renderer manages the scene directly

**Transform** (`Src/Graphics/Scene/Transform.h`)
- Transform data structure for position, rotation (quaternion), scale
- Capabilities: Compute local transform matrix, compose with parent transform for world matrix

### Rendering Pipeline

**Two-Pass Strategy:**
- Pass 1 (Opaque): Collect MeshInstances, group by Material, set PSO once per material, draw all meshes in batch. Benefits: minimal PSO switches, early Z rejection.
- Pass 2 (Transparent): Collect MeshInstances, sort by depth (far-to-near), render in sorted order for correct alpha blending.

**Material Batching:**
- BuildRenderQueue traverses scene graph, collects MeshInstances into material buckets (opaque) or depth-sorted queue (transparent)
- Renderer sets PSO once per material for opaque objects, draws all meshes in that batch, then renders transparent objects depth-sorted

### Dynamic Mesh Editing

**Update Mechanism:**
- CPU-side editing modifies vertex data in upload buffer
- Mark mesh as dirty when vertices change
- UpdateMeshes helper uploads dirty meshes to GPU buffers
- BuildRenderQueue runs each frame, always reflects current state
- Existing constant buffer updates remain unchanged

### Integration

- New `SceneNode` class works with extended `MeshInstance`
- `MeshInstance` supports both standalone use (existing) and scene graph use (new)
- Update `Renderer` to work with root `SceneNode` instead of single `MeshInstance`
- Helper functions for scene operations (traversal, queue building, mesh updates)

### File Structure
```
Src/Graphics/
├── Scene/
│   ├── SceneNode.h/cpp
│   └── Transform.h/cpp
└── Mesh/
    ├── Mesh.h (existing)
    └── MeshInstance.h/cpp (extended)
```

### Implementation Phases
1. **MeshInstance Extension**: Add optional vertex buffers, dirty flag, depth/transparency flags, modify Draw to support PSO skipping, add UpdateMesh method
2. **Core Structure**: Transform, SceneNode (references MeshInstance), world transform computation
3. **Renderer Updates**: Update Renderer to work with root SceneNode, add helper functions for BuildRenderQueue and UpdateMeshes
4. **Rendering**: Two-pass rendering with material batching, Renderer integration with scene graph
5. **Dynamic Mesh Support**: Complete UpdateMesh implementation, UpdateMeshes helper, procedural vertex testing
6. **Blender Integration (Future)**: Scene importer, object/mesh/material mapping

### Performance Notes
- Per-frame batch building: acceptable cost for dynamic meshes (<1ms for thousands of objects)
- Material hash map: O(1) insertion, no sorting for opaque objects
- Depth sorting: only for transparent objects (usually fewer)
- World transform caching: cache matrices, recompute only when local transform changes
- Dirty flags: only update meshes that changed
