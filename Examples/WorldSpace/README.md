# WorldSpace Example

## Overview

This example demonstrates world space transformations and rotation in the DXStarter framework. It ensures that all
parent node transformations are properly applied to child nodes. The example renders three triangles in a scene graph
hierarchy, demonstrating how transformations accumulate through parent-child relationships.

## What It Showcases

1. **Basic Device Initialization**: Creates a DirectX 12 device with debug layers enabled
2. **Shader Loading**: Loads compiled shader bytecode from the materials directory:
    - `WorldPosition.vertx.cso` - Vertex shader that applies world transformation matrices
    - `ColorRed.pixel.cso` - Pixel shader that outputs a solid red color
    - `WorldPosition.rsign.cso` - Root signature for the shader pipeline
3. **Material Creation**: Uses `MaterialBuilder` to create a material from shader bytecode
4. **Mesh Creation**: Creates a simple triangle mesh with 3 vertices in local space
5. **Scene Graph**: Creates a scene with two nodes, each containing the same triangle mesh but with different
   transformations
6. **World Transformations**: Demonstrates translation and rotation transformations:
    - First triangle: Translated to (-0.3, 0.0, 0.0) with no rotation
    - Second triangle: Translated to (0.3, 0.0, 0.0) and rotated 270 degrees around Z-axis
7. **Renderer Setup**: Initializes the renderer and sets the scene
8. **Window Management**: Creates and manages the main application window

## Key Concepts Demonstrated

- **World Space Rendering**: The triangles are rendered using world transformation matrices
- **Transform Operations**: Shows how to use `Node::GetTransform()` to translate and rotate objects
- **Scene Graph Hierarchy**: Demonstrates creating a parent-child node hierarchy where transformations accumulate
  through the tree
- **Shared Geometry**: All three triangles share the same mesh data but are rendered with different accumulated
  transformations
- **Material System**: Shows how to use `MaterialBuilder` to create materials from shader bytecode

## Triangle Geometry

The triangle consists of three vertices:

- Point A: (-0.1, -0.1, 0.0)
- Point B: (0.0, 0.1, 0.0)
- Point C: (0.1, -0.1, 0.0)

Both triangles use the same geometry but are positioned and oriented differently:

- **Straight Triangle**: Positioned at (-0.3, 0.0, 0.0) with default orientation
- **Rotated Triangle**: Positioned at (0.3, 0.0, 0.0) and rotated 270 degrees around the Z-axis

Both triangles are rendered in red using the `ColorRed` pixel shader.

