#pragma once

#include <vector>

#include <SFML/Graphics.hpp>
#ifdef QUADTREES_ENABLE_OPENCL
#include <CL/cl.h>
#endif
#include "QuadTreeNode.h"
#include "Vector2d.h"
#include "Particle.h"


enum class ComputeBackend {
    Sequential,
    OpenMP,
#ifdef QUADTREES_ENABLE_OPENCL
    OpenCL
#endif
};

class QuadTree
{
    std::vector<QuadTreeNode> _nodes;
    std::vector<Particle>& _particles;
    ComputeBackend _backend;
#ifdef QUADTREES_ENABLE_OPENCL
    cl_context _context = nullptr;
    cl_command_queue _queue = nullptr;
    cl_mem _particleBuffer = nullptr;
    cl_mem _nodeBuffer = nullptr;
    size_t _globalDimensions = 0;
    cl_kernel _updatePositionsKernel = nullptr;
    cl_kernel _computeForcesKernel = nullptr;
    void checkError(cl_int status, cl_program program, cl_device_id device);
    cl_kernel buildKernel(cl_device_id device, const char* sourceFile, const char* functionName);
    void initOpenCL();
#endif
    float2 totalForceOn(int particleIdx) const;
    void computeForcesCpu();
    void updatePositionsCpu(float dt);
public:
    QuadTree(std::vector<Particle>& particles, ComputeBackend backend = ComputeBackend::OpenMP);
    ~QuadTree();
    QuadTree(const QuadTree&) = delete;
    QuadTree& operator=(const QuadTree&) = delete;

    ComputeBackend backend() const { return _backend; }
    void computeBounds();
    void build();
    void calculateMass();
    void computeForces();
    void updatePositions(float dt);
    void step(float dt);
    void draw(sf::RenderWindow& window, bool transform = false, bool showGrid = false, bool showMassCenter = false);
};
