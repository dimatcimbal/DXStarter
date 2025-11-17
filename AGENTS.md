# DXStarter Project Agents Configuration

## Project Overview
DXStarter is a DirectX 12 graphics engine starter project built with C++ and CMake. This configuration helps AI agents understand the project's context, build system, and code quality guidelines.

---

## For Code Reviews

This section contains instructions for code review agents regarding code quality guidelines and review focus areas.

### Review Mode: Analysis Only

**IMPORTANT**: When performing code reviews, **DO NOT modify or change any code**. Reviews should be analysis-only, identifying issues and providing recommendations without making actual code changes.

### Primary Review Focus Areas

When reviewing code in both `./Src` and `./Examples` directories, pay the **most attention** to the following areas:

#### 1. Resource Management & Resource Lifetime

- **Ownership Semantics**: Verify that resource ownership is clearly defined and properly managed
- **Lifetime Guarantees**: Check that objects outlive their usage - identify potential dangling pointers or references
- **RAII Compliance**: Ensure resources are properly acquired and released using RAII principles
- **DirectX Resource Lifecycle**: Verify proper management of COM objects, command lists, buffers, and other DirectX resources
- **Resource Leaks**: Identify potential memory leaks, especially with DirectX resources that require explicit cleanup
- **C++ Best Practices**: Follow RAII principles, use smart pointers appropriately, prefer const correctness, use proper initialization lists, and avoid raw pointers when possible

#### 2. Pointer and Reference Usage

Follow modern C++ best practices with this preference order: **References > `std::unique_ptr` > `std::shared_ptr` > Raw Pointers**

Assess the correct usage of:

- **References (`T&` and `const T&`)** - **PREFERRED for non-owning relationships**:
  - Use for non-owning, non-nullable relationships where lifetime is guaranteed
  - Prefer `const T&` for read-only access to avoid unnecessary copies
  - Use `T&` for mutable non-owning references
  - Verify that referenced objects outlive their usage
  - Identify cases where raw pointers are used but references would be more appropriate

- **`std::unique_ptr<T>`** - **PREFERRED for exclusive ownership**:
  - Default choice for single ownership of dynamically allocated resources
  - Verify proper use for exclusive ownership semantics
  - Check for correct move semantics - no unnecessary copies or transfers
  - Ensure proper transfer of ownership using `std::move()` when appropriate
  - Verify that `unique_ptr` is not being copied (should use move semantics)
  - Prefer `std::make_unique<T>()` over `new T` for exception safety

- **`std::shared_ptr<T>`** - **Use sparingly, only when shared ownership is required**:
  - Verify use only when multiple objects truly need shared ownership
  - Check for circular reference risks - use `std::weak_ptr<T>` to break cycles
  - Ensure proper use in registries, caches, or observer patterns where multiple owners exist
  - Prefer `std::make_shared<T>()` over `new T` for efficiency and exception safety
  - Avoid overuse - shared ownership adds overhead and complexity
  - Consider if `std::unique_ptr` with proper lifetime management would suffice

- **`std::weak_ptr<T>`** - **For breaking circular references**:
  - Verify use when breaking circular dependencies with `std::shared_ptr`
  - Check that `weak_ptr` is properly converted to `shared_ptr` before use
  - Ensure proper use in observer patterns or caches where objects may be destroyed

- **`std::optional<T>`** - **For nullable value semantics**:
  - Consider if nullable values should use `std::optional<T>` instead of raw pointers
  - Prefer `std::optional<T>` over `T*` when nullability represents "no value" rather than "no object"
  - Verify proper use of `has_value()` and `value()` or `value_or()`

- **Raw Pointers (`T*`)** - **Use only when necessary**:
  - Acceptable for non-owning, nullable relationships (observer patterns, optional parameters)
  - Verify lifetime guarantees are clearly documented and enforced
  - Check if raw pointers should be replaced with references (non-nullable) or `std::optional` (nullable values)
  - Look for potential dangling pointer scenarios
  - Consider if `std::observer_ptr<T>` (C++23) or similar would be more explicit
  - Avoid raw pointers for ownership - use smart pointers instead
  - Document why raw pointers are necessary when used

**General Guidelines:**

- **Prefer value semantics** when objects are small and copying is cheap
- **Use const correctness** - prefer `const T&` for read-only parameters
- **Parameter passing**: Use `const T&` for read-only, `T&&` for move-only, `T*` only when nullability is required
- **Return values**: Return by value for small types, by reference/pointer for large types or when returning existing objects
- **Ownership transfer**: Use `std::move()` explicitly when transferring ownership of `unique_ptr` or move-only types

#### 3. Excessive Copying

- **Unnecessary Copies**: Identify places where objects are copied when they could be moved or passed by reference
- **Large Object Copies**: Check for copying of large objects (meshes, buffers, matrices) that should use move semantics
- **Parameter Passing**: Verify parameters are passed efficiently (const reference for read-only, move for transfer of ownership)
- **Return Value Optimization**: Check for opportunities to use move semantics in return values

#### 4. Move Semantics Verification

When reviewing classes with move constructors and move assignment operators, verify that moved-from objects are properly reset to a valid but unspecified state. This is critical for preventing use-after-move bugs and ensuring correct resource management.

**Key Verification Points:**

- **Raw Pointers (`T*`)**: Must be set to `nullptr` in moved-from objects
  - Use `std::exchange(other.mPointer, nullptr)` in move constructor
  - Use `mPointer = std::exchange(other.mPointer, nullptr);` in move assignment
  - Verify both move constructor and move assignment operator reset pointers

- **Smart Pointers (`std::unique_ptr<T>`, `std::shared_ptr<T>`)**: Should be moved using `std::move()` or `std::exchange()`
  - Use `std::exchange(other.mUniquePtr, nullptr)` to explicitly reset
  - Or use `std::move(other.mUniquePtr)` which automatically resets the source
  - Verify the moved-from smart pointer is in a null state

- **COM Pointers (`ComPtr<T>`)**: Should be moved using `std::move()` or `std::exchange()`
  - Use `std::exchange(other.mComPtr, nullptr)` to explicitly reset
  - Or use `std::move(other.mComPtr)` which automatically resets the source
  - Verify the moved-from ComPtr is in a null state

- **Value Types**: Should be reset to default/zero state where appropriate
  - Use `std::exchange(other.mValue, 0)` for numeric types
  - Use `std::exchange(other.mValue, {})` for structs/classes with default constructors
  - Use `std::exchange(other.mValue, Type{})` for explicit default construction
  - For value types that don't need resetting (e.g., small POD types), copying is acceptable

- **Containers (`std::vector`, `std::set`, etc.)**: Should be moved or reset to empty
  - Use `std::move(other.mContainer)` or `std::exchange(other.mContainer, {})`
  - Verify the moved-from container is empty

**Common Patterns:**

✅ **Correct Pattern - Raw Pointer:**
```cpp
// Move constructor
MyClass(MyClass&& other) noexcept
    : mPointer(std::exchange(other.mPointer, nullptr)) {}

// Move assignment
MyClass& operator=(MyClass&& other) noexcept {
    if (this != &other) {
        mPointer = std::exchange(other.mPointer, nullptr);
    }
    return *this;
}
```

✅ **Correct Pattern - Smart Pointer:**
```cpp
// Move constructor
MyClass(MyClass&& other) noexcept
    : mUniquePtr(std::exchange(other.mUniquePtr, nullptr)) {}

// Move assignment
MyClass& operator=(MyClass&& other) noexcept {
    if (this != &other) {
        mUniquePtr = std::exchange(other.mUniquePtr, nullptr);
    }
    return *this;
}
```

✅ **Correct Pattern - Value Type:**
```cpp
// Move constructor
MyClass(MyClass&& other) noexcept
    : mValue(std::exchange(other.mValue, 0)),
      mMatrix(std::exchange(other.mMatrix, Matrix4{})) {}

// Move assignment
MyClass& operator=(MyClass&& other) noexcept {
    if (this != &other) {
        mValue = std::exchange(other.mValue, 0);
        mMatrix = std::exchange(other.mMatrix, Matrix4{});
    }
    return *this;
}
```

❌ **Incorrect Pattern - Missing Reset:**
```cpp
// BAD: Pointer not reset in moved-from object
MyClass(MyClass&& other) noexcept
    : mPointer(other.mPointer) {}  // Missing std::exchange!

// BAD: Value type incorrectly reset
MyClass(MyClass&& other) noexcept
    : mMatrix(std::exchange(other.mMatrix, nullptr)) {}  // nullptr for value type!
```

**Checklist for Move Semantics Review:**

- [ ] All raw pointers are set to `nullptr` in moved-from objects
- [ ] All smart pointers are properly moved/reset in moved-from objects
- [ ] All COM pointers are properly moved/reset in moved-from objects
- [ ] Value types are reset to appropriate default values (not `nullptr` unless it's a pointer)
- [ ] Containers are moved or reset to empty state
- [ ] Both move constructor and move assignment operator are checked
- [ ] Self-assignment check (`if (this != &other)`) is present in move assignment
- [ ] Move operations are marked `noexcept` where appropriate
- [ ] Base class members are properly handled in derived class move operations

### Code Quality Focus Areas

- **Error Handling**: Verify proper HRESULT checking and error propagation
- **Performance**: Look for potential performance bottlenecks in rendering pipeline
- **Pipeline Optimization**: Verify efficient rendering by grouping draw calls to reuse the same material, avoiding unnecessary PSO (Pipeline State Object) and RootSignature switching, and minimizing state changes
- **Thread Safety**: Ensure proper synchronization for multi-threaded DirectX operations

**Note**: Memory management, resource leaks, and C++ best practices are covered in the Primary Review Focus Areas above.

### DirectX 12 Specific Checks

- **Command Lists**: Verify proper command list recording and execution
- **Descriptor Heaps**: Check descriptor heap management
- **Synchronization**: Ensure proper synchronization between CPU and GPU
- **Resource States**: Validate resource state transitions
- **Swap Chain**: Review swap chain management

### Areas to Pay Special Attention To

- Graphics pipeline setup and management
- Resource upload and management
- Command queue and allocator usage
- Mesh and rendering data structures
- Window management and message handling

### Areas That Are Less Critical

- Build artifacts and generated files
- Third-party dependencies (vcpkg managed)
- Documentation formatting (unless it affects functionality)
- CMake configuration (unless it affects functionality)

---

## For Cursor

This section contains instructions for Cursor AI agents regarding formatting, DEVLOG.md generation, and local build operations.

### Build Commands

#### Debug Build
```powershell
.\keeper.ps1 -Build -Config Debug
```
Builds the project in Debug configuration with debug symbols and no optimizations.

#### Release Build
```powershell
.\keeper.ps1 -Build -Config Release
```
Builds the project in Release configuration with optimizations enabled.

#### Rebuild (Clean + Build)
```powershell
.\keeper.ps1 -Rebuild -Config Debug
.\keeper.ps1 -Rebuild -Config Release
```
Cleans the build directory and performs a fresh build. This is the default action if no action is specified.

### Maintenance Commands

#### Clean Build Artifacts
```powershell
.\keeper.ps1 -Clean
```
Removes all build artifacts and temporary files to ensure a clean build state.

#### Code Formatting
```powershell
.\keeper.ps1 -Format
```
Formats the source code according to the project's coding standards.

#### Check Code Formatting
```powershell
.\keeper.ps1 -CheckFormat
```
Checks if source code is properly formatted without making changes. Returns an error if formatting issues are found.

#### Generate Project Files
```powershell
.\keeper.ps1 -Generate
```
Generates CMake project files without building. Useful for IDE integration.

#### Install Dependencies
```powershell
.\keeper.ps1 -Deps
```
Installs vcpkg dependencies specified in `vcpkg.json`.

### Build Workflow Guidelines

- Always run `-Clean` before switching between Debug and Release builds
- Use Debug builds for development and debugging
- Use Release builds for performance testing and final distribution
- Use `-Rebuild` for a clean build (default action when no action is specified)
- All build commands support `-Config Debug` or `-Config Release` (Debug by default)
- Run `-Format` before committing code to maintain consistent style
- Use `-CheckFormat` in CI/CD pipelines or pre-commit hooks to verify formatting
- Use `-All` to format, generate, and build in one command for quick workflows
- The `keeper.ps1` script handles CMake configuration, vcpkg dependencies, and build orchestration

### Project Structure Context

- **Build System**: CMake-based with Visual Studio 2022+ generator
- **Dependency Management**: vcpkg for package management
- **Graphics API**: DirectX 12 graphics engine
- **Language**: C++ with Windows-specific code
- **Build Output**: Generated files and executables go to `build/` directory

### Housekeeper Submodule Management

The `Housekeeper/` directory is a git submodule. The actual Housekeeper repository is located at `C:\Users\dtcimbal\code\Housekeeper`.

**Workflow for updating Housekeeper:**
1. Make changes in the actual repository: `C:\Users\dtcimbal\code\Housekeeper`
2. Commit and push changes to the Housekeeper repository
3. Update the submodule in DXStarter:
   ```powershell
   cd Housekeeper
   git fetch origin
   git checkout main  # or the branch/commit you want
   git pull
   cd ..
   git add Housekeeper  # Stage the submodule update
   ```
4. The submodule reference will be updated in DXStarter's commit

**Note**: Always work in the actual Housekeeper repository, not the submodule directory, when making changes to the build script.

### DEVLOG.md Structure and Format

The `DEVLOG.md` file serves as a changelog documenting all pull requests and significant changes to the project. It follows a consistent structure:

#### Entry Format

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

#### Example Structure

```markdown
## [#19 Mesh classes](https://github.com/dimatcimbal/DXStarter/pull/19)

Added mesh classes for instance-based rendering.

1. **Mesh Classes**:
   * Created `Mesh` class to hold vertex buffer data.
   * Added `MeshInstance` class for rendering multiple copies...
   
2. **Application Integration**:
   * Updated `Main.cpp` to showcase mesh creation workflow...
```

#### Devlog Guidelines

- Entries are ordered chronologically (newest first)
- Use clear, descriptive category names
- List specific changes with bullet points
- Reference class names, methods, and files using backticks (e.g., `` `Device::Create()` ``)
- Include technical details about implementation when relevant
- Link to the corresponding GitHub pull request in the header

**Simplification Rules:**

Omit the following from devlog entries:

- **Shader functionality**: Only mention that shader files were created, not what they do
- **Move semantics**: Omit references to move semantics support unless it's a significant architectural change
- **Insignificant changes**: Skip minor implementation details like pointer-to-reference conversions, parameter type changes, or similar syntactic changes
- **Pipeline configuration details**: Avoid listing specific rasterizer, blend state, depth/stencil, or other pipeline state configuration details unless they represent significant architectural decisions
- **Accessor/getter methods**: Omit mentions of accessor or getter method additions as they are implementation details
- **Const correctness**: Omit mentions of making methods const as this is an insignificant change
- **Code cleanup**: Skip mentions of code cleanup activities like removing unused includes, updating include paths, fixing typos, or similar maintenance tasks

#### Getting Changes for the Devlog
```powershell
git --no-pager diff --staged --stat
```
Shows staged changes summary in a format suitable for reviewing changes when writing new entries in `DEVLOG.md`. Use `git --no-pager diff --staged` for full diff output.
