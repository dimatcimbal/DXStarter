# PainTriangle Example

## Overview

This example demonstrates the most basic rendering setup in the DXStarter framework. It renders a single red triangle
using the simplest possible configuration.

## What It Showcases

1. **Basic Device Initialization**: Creates a DirectX 12 device with debug layers enabled
2. **Shader Loading**: Loads compiled shader bytecode from the materials directory:
    - `LocalPosition.vertx.cso` - Vertex shader that uses local coordinates (no world transformation)
    - `ColorRed.pixel.cso` - Pixel shader that outputs a solid red color
    - `Base.rsign.cso` - Root signature for the shader pipeline
3. **Material Creation**: Uses `MaterialBuilder` to create a material from shader bytecode
4. **Mesh Creation**: Creates a simple triangle mesh with 3 vertices in local space
5. **Scene Graph**: Creates a single scene node containing the triangle mesh
6. **Renderer Setup**: Initializes the renderer and sets the scene
7. **Window Management**: Creates and manages the main application window

## Key Concepts Demonstrated

- **Local Space Rendering**: The triangle is rendered in local coordinates without any world transformation
- **Material System**: Shows how to use `MaterialBuilder` to create materials from shader bytecode
- **Basic Scene Graph**: Demonstrates creating a simple scene with a single node
- **Framework Initialization**: Shows the complete initialization sequence for a minimal DirectX 12 application

## Triangle Geometry

The triangle consists of three vertices:

- Point A: (-0.1, -0.1, 0.0)
- Point B: (0.0, 0.1, 0.0)
- Point C: (0.1, -0.1, 0.0)

The triangle is rendered in red using the `ColorRed` pixel shader.

