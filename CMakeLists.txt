cmake_minimum_required(VERSION 3.24)
project(quadtrees LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(ENABLE_OPENCL "Build the OpenCL GPU backend" ON)

find_package(OpenMP REQUIRED)

set(SFML_LIBS)
find_package(SFML 3 CONFIG COMPONENTS Graphics Window System QUIET)
if(SFML_FOUND)
    set(SFML_LIBS SFML::Graphics SFML::Window SFML::System)
else()
    find_package(SFML COMPONENTS graphics window system REQUIRED)
    set(SFML_LIBS sfml-graphics sfml-window sfml-system)
endif()

set(OPENCL_LIBS)
if(ENABLE_OPENCL)
    find_package(OpenCL QUIET)
    if(OpenCL_FOUND)
        set(OPENCL_LIBS OpenCL::OpenCL)
    else()
        message(WARNING "OpenCL not found; building CPU backends only (seq, OpenMP)")
        set(ENABLE_OPENCL OFF)
    endif()
endif()

add_executable(quadtrees
    quadtrees/main.cpp
    quadtrees/Particle.cpp
    quadtrees/QuadTree.cpp
    quadtrees/QuadTreeNode.cpp
    quadtrees/Vector2d.cpp
)

target_link_libraries(quadtrees PRIVATE
    ${SFML_LIBS}
    OpenMP::OpenMP_CXX
    ${OPENCL_LIBS}
)

if(ENABLE_OPENCL)
    target_compile_definitions(quadtrees PRIVATE
        CL_TARGET_OPENCL_VERSION=300
        QUADTREES_ENABLE_OPENCL=1
    )
endif()

# Kernels are opened as "kernels/*.cl" relative to the process working directory.
add_custom_command(TARGET quadtrees POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/quadtrees/kernels
        $<TARGET_FILE_DIR:quadtrees>/kernels
)
