# MaterialBatching Example

## Overview

This example demonstrates material batching and efficient rendering of multiple objects with different materials in the
DXStarter framework. It renders 30 triangles with alternating red and blue materials, randomly positioned across the
screen.

## What It Showcases

1. **Basic Device Initialization**: Creates a DirectX 12 device with debug layers enabled
2. **Multiple Material Creation**: Creates two different materials (Red and Blue) using the same vertex shader but
   different pixel shaders:
    - `WorldPosition.vertx.cso` - Vertex shader that applies world transformation matrices (shared by both materials)
    - `ColorRed.pixel.cso` - Pixel shader that outputs a solid red color
    - `ColorBlue.pixel.cso` - Pixel shader that outputs a solid blue color
    - `WorldPosition.rsign.cso` - Root signature for the shader pipeline
3. **Material System**: Uses `MaterialBuilder` to create multiple materials from shader bytecode
4. **Shared Mesh Geometry**: Creates a single triangle mesh that is reused by all 30 triangle instances
5. **Scene Graph**: Creates a scene with 30 nodes, each containing the same triangle mesh but with different materials
   and transformations
6. **Random Positioning**: Uses random number generation to position triangles randomly across the screen (from -1.0 to
   1.0 in both X and Y)
7. **Material Alternation**: Alternates between red and blue materials for each triangle
8. **Renderer Setup**: Initializes the renderer and sets the scene
9. **Window Management**: Creates and manages the main application window

## Key Concepts Demonstrated

- **Material Batching**: Shows how to efficiently render multiple objects with different materials
- **Shared Geometry**: Demonstrates reusing the same mesh data for multiple instances
- **World Space Rendering**: The triangles are rendered using world transformation matrices
- **Scene Graph Hierarchy**: Demonstrates creating multiple nodes in a scene with different materials and
  transformations
- **Material System**: Shows how to create and use multiple materials with `MaterialBuilder`
- **Efficient Rendering**: Demonstrates how the renderer batches draw calls by material to minimize state changes

## Triangle Geometry

The triangle consists of three vertices:

- Point A: (-0.1, -0.1, 0.0)
- Point B: (0.0, 0.1, 0.0)
- Point C: (0.1, -0.1, 0.0)

All 30 triangles use the same geometry but are positioned randomly and use alternating materials:

- **Red Triangles**: Even-indexed triangles (0, 2, 4, ...) use the `ColorRed` pixel shader
- **Blue Triangles**: Odd-indexed triangles (1, 3, 5, ...) use the `ColorBlue` pixel shader
- **Random Positions**: Each triangle is positioned randomly between -1.0 and 1.0 in both X and Y coordinates

