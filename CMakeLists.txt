cmake_minimum_required(VERSION 3.10)

# this is our project name
project(DXStarter VERSION 0.1.0)

# config section below
set(CMAKE_CXX_STANDARD 20) # enforce /std:c++20
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
# end of config

# source files glob src/*{.h|.cpp}
file(GLOB_RECURSE SRCS
    "${SRC_DIR}/*.cpp"
    "${SRC_DIR}/*.h"
)

# VCPKG dependencies
find_package(directx-headers CONFIG REQUIRED)
find_path(D3DX12_INCLUDE_DIR "d3dx12.h")

# executable with the name as the project name
add_executable(${PROJECT_NAME} WIN32 ${SRCS})

# configure includes
target_include_directories(${PROJECT_NAME} PRIVATE
    ${SRC_DIR}
    ${D3DX12_INCLUDE_DIR}
)

# Use vcpkg targets for proper linking
target_link_libraries(${PROJECT_NAME} PRIVATE
    Microsoft::DirectX-Headers
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    # user input, window management, etc.
    user32
    
    # DX dependencies
    d3d12
    
    # DXGI for swap chain and other utilities
    dxgi

    # DirectX debug layer
    dxguid
)

# compile with NOMINMAX and UNICODE,_UNICODE defined
target_compile_definitions(${PROJECT_NAME} PRIVATE
    # Exclude min and max macro
    NOMINMAX 
    # Define UNICODE and _UNICODE to use wide-character versions of WinAPI functions in the macros like OutputDebugString
    UNICODE _UNICODE
)
