# DXStarter Project Skills Configuration

## Build Commands

### Debug Build
```powershell
.\keeper.ps1 -Build -Config=Debug
```
Builds the project in Debug configuration with debug symbols and no optimizations.

### Release Build
```powershell
.\keeper.ps1 -Build -Config=Release
```
Builds the project in Release configuration with optimizations enabled.

## Maintenance Commands

### Clean Build Artifacts
```powershell
.\keeper.ps1 -Clean
```
Removes all build artifacts and temporary files to ensure a clean build state.

### Code Formatting
```powershell
.\keeper.ps1 -Format
```
Formats the source code according to the project's coding standards.

## Usage Guidelines

- Always run `-Clean` before switching between Debug and Release builds
- Use Debug builds for development and debugging
- Use Release builds for performance testing and final distribution
- Run `-Format` before committing code to maintain consistent style
- The keeper.ps1 script handles CMake configuration, vcpkg dependencies, and build orchestration

## Project Structure Context
- CMake-based build system
- Uses vcpkg for dependency management
- DirectX 12 graphics engine
- C++ project with Windows-specific code
- Build output goes to `build/` directory
