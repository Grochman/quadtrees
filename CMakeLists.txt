cmake_minimum_required(VERSION 3.24)
project(quadtrees LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(SFML 3 CONFIG REQUIRED COMPONENTS Graphics Window System)
find_package(OpenCL REQUIRED)

add_executable(quadtrees
    src/main.cpp
    src/QuadTree.cpp
    src/ComputeBackend.cpp
    src/Renderer.cpp
)

target_include_directories(quadtrees PRIVATE src)
target_compile_definitions(quadtrees PRIVATE CL_TARGET_OPENCL_VERSION=300)

target_link_libraries(quadtrees PRIVATE
    SFML::Graphics
    SFML::Window
    SFML::System
    OpenCL::OpenCL
)
