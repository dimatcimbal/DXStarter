cmake_minimum_required(VERSION 3.10)

# this is our project name
project(DXStarter VERSION 0.1.0)

# config section below
set(CMAKE_CXX_STANDARD 20) # enforce /std:c++20
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(EXAMPLES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Examples)
set(MATERIALS_DIR ${EXAMPLES_DIR}/Materials)
# end of config

# --- Framework Library ---

# source files glob src/*{.h|.cpp}
file(GLOB_RECURSE FRAMEWORK_SRCS
    "${SRC_DIR}/*.cpp"
    "${SRC_DIR}/*.h"
)

# VCPKG dependencies
find_package(directx-headers CONFIG REQUIRED)
find_path(D3DX12_INCLUDE_DIR "d3dx12.h")

# Create framework library
add_library(DXFramework STATIC ${FRAMEWORK_SRCS})

# configure includes for framework
target_include_directories(DXFramework PUBLIC
    ${SRC_DIR}
    ${D3DX12_INCLUDE_DIR}
)

# Use vcpkg targets for proper linking
target_link_libraries(DXFramework PUBLIC
    Microsoft::DirectX-Headers
)

target_link_libraries(DXFramework PUBLIC
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
target_compile_definitions(DXFramework PUBLIC
    # Exclude min and max macro
    NOMINMAX 
    # Define UNICODE and _UNICODE to use wide-character versions of WinAPI functions in the macros like OutputDebugString
    UNICODE _UNICODE
)

# Enable fast floating-point model for all builds
target_compile_options(DXFramework PUBLIC
    $<$<CXX_COMPILER_ID:MSVC>:/fp:fast>
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

# Collect all shaders from the shared Materials directory
file(GLOB_RECURSE ALL_SHADERS "${MATERIALS_DIR}/*.hlsl")

# Create a single shared shader compilation target
set(SHARED_SHADER_TARGET_NAME SharedShaders)
add_custom_target(${SHARED_SHADER_TARGET_NAME} ALL)

foreach(SHADER ${ALL_SHADERS})
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
    
    # Check shader type based on filename pattern: *.rsign.hlsl, *.pixel.hlsl, *.vertx.hlsl
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
    # Compile to shared location - use generator expression directly in OUTPUT
    set(SHARED_OUT_CSO "${CMAKE_BINARY_DIR}/$<CONFIG>/Materials/${SHADER_BASE_NAME}${SHADER_TYPE_SUFFIX}.cso")

    add_custom_command(
        OUTPUT ${SHARED_OUT_CSO}
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/$<CONFIG>/Materials"
        COMMAND ${DXC_EXE}
            -T ${SHADER_PROFILE_DXC}
            -E ${SHADER_ENTRY}
            -Fo ${SHARED_OUT_CSO}
            ${SHADER}
        DEPENDS ${SHADER}
        BYPRODUCTS ${SHARED_OUT_CSO}
        COMMENT "Compiling HLSL (dxc): ${SHADER_NAME} -> ${SHARED_OUT_CSO}"
        VERBATIM
    )

    # Create per-shader target
    set(SHADER_TARGET_NAME ${SHARED_SHADER_TARGET_NAME}_${SHADER_BASE_NAME}${SHADER_TYPE_SUFFIX})
    add_custom_target(${SHADER_TARGET_NAME} DEPENDS ${SHARED_OUT_CSO})
    add_dependencies(${SHARED_SHADER_TARGET_NAME} ${SHADER_TARGET_NAME})
endforeach()


# --- Example Projects ---

# Find all example directories
file(GLOB EXAMPLE_DIRS "${EXAMPLES_DIR}/*")
list(FILTER EXAMPLE_DIRS INCLUDE REGEX ".*")

foreach(EXAMPLE_DIR ${EXAMPLE_DIRS})
    # Check if it's a directory
    if(NOT IS_DIRECTORY ${EXAMPLE_DIR})
        continue()
    endif()
    
    # Get example name from directory name
    get_filename_component(EXAMPLE_NAME ${EXAMPLE_DIR} NAME)
    
    # Find source files in the example directory
    # Support both "Src" and "src" directory names
    set(EXAMPLE_SRC_PATHS
        "${EXAMPLE_DIR}/Src"
        "${EXAMPLE_DIR}/src"
    )
    
    set(EXAMPLE_SRCS "")
    foreach(SRC_PATH ${EXAMPLE_SRC_PATHS})
        if(IS_DIRECTORY ${SRC_PATH})
            file(GLOB_RECURSE FOUND_SRCS
                "${SRC_PATH}/*.cpp"
                "${SRC_PATH}/*.h"
            )
            if(FOUND_SRCS)
                list(APPEND EXAMPLE_SRCS ${FOUND_SRCS})
            endif()
        endif()
    endforeach()
    
    if(NOT EXAMPLE_SRCS)
        # No source files found, skip this example
        continue()
    endif()
    
    # Create executable for this example
    add_executable(${EXAMPLE_NAME} WIN32 ${EXAMPLE_SRCS})
    
    # Set output directory for executable (so shaders can be placed next to it)
    # Use set_property to allow generator expressions
    set_property(TARGET ${EXAMPLE_NAME} PROPERTY
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>/${EXAMPLE_NAME}"
    )
    
    # Link to framework library
    target_link_libraries(${EXAMPLE_NAME} PRIVATE DXFramework)
    
    # Ensure shaders are compiled before the example builds
    add_dependencies(${EXAMPLE_NAME} ${SHARED_SHADER_TARGET_NAME})
endforeach()
