# Bugbot Configuration for DXStarter

## Project Overview
DXStarter is a DirectX 12 graphics engine starter project built with C++ and CMake. This configuration helps Bugbot understand the project's context and focus areas for code reviews.

## General Review Guidelines

### Code Quality Focus Areas
- **Memory Management**: Check for proper DirectX resource management and COM object lifecycle
- **Error Handling**: Verify proper HRESULT checking and error propagation
- **Performance**: Look for potential performance bottlenecks in rendering pipeline
- **Thread Safety**: Ensure proper synchronization for multi-threaded DirectX operations
- **Resource Leaks**: Identify potential memory leaks, especially with DirectX resources

### C++ Best Practices
- Follow RAII principles for resource management
- Use smart pointers where appropriate
- Prefer const correctness
- Use proper initialization lists
- Avoid raw pointers when possible

### DirectX 12 Specific Checks
- Verify proper command list recording and execution
- Check descriptor heap management
- Ensure proper synchronization between CPU and GPU
- Validate resource state transitions
- Review swap chain management

### CMake and Build System
- Verify proper target dependencies
- Check for correct include directories
- Ensure proper library linking
- Validate compiler flags and definitions

## Areas to Pay Special Attention To
- Graphics pipeline setup and management
- Resource upload and management
- Command queue and allocator usage
- Mesh and rendering data structures
- Window management and message handling

## Areas That Are Less Critical
- Build artifacts and generated files
- Third-party dependencies (vcpkg managed)
- Documentation formatting (unless it affects functionality)
