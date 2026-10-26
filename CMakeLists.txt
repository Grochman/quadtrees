cmake_minimum_required(VERSION 3.24)
project(quadtrees LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SFML 3 CONFIG REQUIRED COMPONENTS Graphics Window System)
find_package(OpenCL REQUIRED)

add_executable(quadtrees
    quadtrees/main.cpp
    quadtrees/Particle.cpp
    quadtrees/QuadTree.cpp
    quadtrees/QuadTreeNode.cpp
    quadtrees/Vector2d.cpp
)

target_compile_definitions(quadtrees PRIVATE CL_TARGET_OPENCL_VERSION=300)

target_link_libraries(quadtrees PRIVATE
    SFML::Graphics
    SFML::Window
    SFML::System
    OpenCL::OpenCL
)

# Kernels are opened as "kernels/*.cl" relative to the process working directory.
add_custom_command(TARGET quadtrees POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/quadtrees/kernels
        $<TARGET_FILE_DIR:quadtrees>/kernels
)
