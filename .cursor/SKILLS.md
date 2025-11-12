# DXStarter Project Skills Configuration

## Build Commands

### Debug Build
```powershell
.\keeper.ps1 -Build -Config Debug
```
Builds the project in Debug configuration with debug symbols and no optimizations.

### Release Build
```powershell
.\keeper.ps1 -Build -Config Release
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

### Rebuild (Clean + Build)
```powershell
.\keeper.ps1 -Rebuild -Config Debug
.\keeper.ps1 -Rebuild -Config Release
```
Cleans the build directory and performs a fresh build. This is the default action if no action is specified.

### Generate Project Files
```powershell
.\keeper.ps1 -Generate
```
Generates CMake project files without building. Useful for IDE integration.

### Check Code Formatting
```powershell
.\keeper.ps1 -CheckFormat
```
Checks if source code is properly formatted without making changes. Returns an error if formatting issues are found.

### Install Dependencies
```powershell
.\keeper.ps1 -Deps
```
Installs vcpkg dependencies specified in `vcpkg.json`.

### Getting Changes for the Devlog
```powershell
git --no-pager diff --cached
```
Shows staged changes in a format suitable for copying into `DEVLOG.md`. Use this to summarize the latest changes when writing new entries in `DEVLOG.md`.

## DEVLOG.md Structure and Format

The `DEVLOG.md` file serves as a changelog documenting all pull requests and significant changes to the project. It follows a consistent structure:

### Entry Format

Each entry in the devlog follows this structure:

1. **Header**: `## [#N Title](link)`
   - `N` is the pull request number
   - `Title` is a brief descriptive title
   - `link` is the GitHub pull request URL

2. **Summary**: A single sentence or brief paragraph describing the overall change

3. **Detailed Sections**: Numbered sections (1., 2., 3., etc.) with:
   - **Bold category names** (e.g., `**Mesh Classes**:`, `**Application Integration**:`)
   - Bullet points (`*`) describing specific changes
   - Nested bullet points for sub-details when needed

### Example Structure

```markdown
## [#19 Mesh classes](https://github.com/dimatcimbal/DXStarter/pull/19)

Added mesh classes for instance-based rendering.

1. **Mesh Classes**:
   * Created `Mesh` class to hold vertex buffer data.
   * Added `MeshInstance` class for rendering multiple copies...
   
2. **Application Integration**:
   * Updated `Main.cpp` to showcase mesh creation workflow...
```

### Devlog Guidelines

- Entries are ordered chronologically (newest first)
- Use clear, descriptive category names
- List specific changes with bullet points
- Reference class names, methods, and files using backticks (e.g., `` `Device::Create()` ``)
- Include technical details about implementation when relevant
- Link to the corresponding GitHub pull request in the header

**Simplification Rules:**
- Don't describe shader functionality: Only mention that shader files were created, not what they do
- Don't mention move semantics explicitly: Omit references to move semantics support unless it's a significant architectural change
- Don't document insignificant changes: Skip minor implementation details like pointer-to-reference conversions, parameter type changes, or similar syntactic changes
- Don't detail pipeline configuration: Avoid listing specific rasterizer, blend state, depth/stencil, or other pipeline state configuration details unless they represent significant architectural decisions

## Usage Guidelines

### Build Workflow
- Always run `-Clean` before switching between Debug and Release builds
- Use Debug builds for development and debugging
- Use Release builds for performance testing and final distribution
- Use `-Rebuild` for a clean build (default action when no action is specified)
- All build commands support `-Config Debug` or `-Config Release` (default: Release)

### Code Quality
- Run `-Format` before committing code to maintain consistent style
- Use `-CheckFormat` in CI/CD pipelines or pre-commit hooks to verify formatting

### Quick Workflows
- Use `-All` to format, generate, and build in one command for quick workflows
- The `keeper.ps1` script handles CMake configuration, vcpkg dependencies, and build orchestration

## Project Structure Context

- **Build System**: CMake-based with Visual Studio 2022+ generator
- **Dependency Management**: vcpkg for package management
- **Graphics API**: DirectX 12 graphics engine
- **Language**: C++ with Windows-specific code
- **Build Output**: Generated files and executables go to `build/` directory
