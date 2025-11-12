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

# --- HLSL shader compilation (requires DXC for root signatures) ---

# Find dxc.exe (Windows SDK). Required for root signature compilation.
find_program(DXC_EXE dxc
    HINTS
        "$ENV{WindowsSdkDir}/bin/$ENV{WindowsSDKVersion}/x64"
        "C:/Program Files (x86)/Windows Kits/10/bin/x64"
        "C:/Program Files (x86)/Windows Kits/10/bin/10.0.*/x64"
    REQUIRED
)

# Collect all shaders to compile
file(GLOB_RECURSE HLSL_SHADERS
    "${CMAKE_SOURCE_DIR}/Resources/Shaders/*.hlsl"
)

# Per-config shader output directory
set(SHADER_OUT_DIR ${CMAKE_BINARY_DIR}/$<CONFIG>/Shaders/)

add_custom_target(Shaders ALL)

foreach(SHADER ${HLSL_SHADERS})
    get_filename_component(SHADER_NAME ${SHADER} NAME_WE)
    get_filename_component(SHADER_FULL_NAME ${SHADER} NAME)
    get_filename_component(SHADER_EXT ${SHADER} EXT)
    
    # Skip .hlsli files (they're headers/includes, not shaders to compile)
    if (SHADER_EXT STREQUAL ".hlsli")
        continue()
    endif()

    # Choose tool and profile per shader based on filename
    set(SHADER_ENTRY main)
    set(IS_ROOT_SIGNATURE FALSE)
    
    # Check shader type based on filename pattern: *.rsign.hlsl, *.pixel.hlsl, *.vertex.hlsl
    if (SHADER_FULL_NAME MATCHES "\\.rsign\\.hlsl$")
        set(IS_ROOT_SIGNATURE TRUE)
        set(SHADER_ENTRY ROOTSIGN)
        set(SHADER_TYPE_SUFFIX ".rsign")
        # Root signature profile
        set(SHADER_PROFILE_DXC rootsig_1_1)
    elseif (SHADER_FULL_NAME MATCHES "\\.pixel\\.hlsl$")
        set(SHADER_TYPE_SUFFIX ".pixel")
        # Pixel shader profiles
        set(SHADER_PROFILE_DXC ps_6_0)
    elseif (SHADER_FULL_NAME MATCHES "\\.vertx\\.hlsl$")
        set(SHADER_TYPE_SUFFIX ".vertx")
        # Vertex shader profiles (supports both .vertx and .vertex)
        set(SHADER_PROFILE_DXC vs_6_0)
    else()
        # Default to vertex shader for backward compatibility (no suffix)
        set(SHADER_TYPE_SUFFIX "")
        set(SHADER_PROFILE_DXC vs_6_0)
    endif()
    
    # Construct output filename with shader type prefix included
    # Extract base name and explicitly append the shader type suffix to ensure it's included
    string(REGEX REPLACE "${SHADER_TYPE_SUFFIX}$" "" SHADER_BASE_NAME ${SHADER_NAME})
    set(OUT_CSO ${SHADER_OUT_DIR}/${SHADER_BASE_NAME}${SHADER_TYPE_SUFFIX}.cso)

    add_custom_command(
        OUTPUT ${OUT_CSO}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADER_OUT_DIR}
        COMMAND ${DXC_EXE}
            -T ${SHADER_PROFILE_DXC}
            -E ${SHADER_ENTRY}
            -Fo ${OUT_CSO}
            ${SHADER}
        DEPENDS ${SHADER}
        BYPRODUCTS ${OUT_CSO}
        COMMENT "Compiling HLSL (dxc): ${SHADER_NAME} -> ${OUT_CSO}"
        VERBATIM
    )

    # Use base name + suffix for unique target names
    set(SHADER_TARGET_NAME ${SHADER_BASE_NAME}${SHADER_TYPE_SUFFIX})
    add_custom_target(Shader_${SHADER_TARGET_NAME} DEPENDS ${OUT_CSO})
    add_dependencies(Shaders Shader_${SHADER_TARGET_NAME})
endforeach()

# Ensure the app waits for shaders to be built
add_dependencies(${PROJECT_NAME} Shaders)