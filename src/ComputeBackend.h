#pragma once

#include "Particle.h"
#include "QuadTreeNode.h"
#include <CL/cl.h>
#include <vector>

class ComputeBackend {
  public:
    ComputeBackend(std::vector<Particle> &particles);
    ~ComputeBackend();
    ComputeBackend(const ComputeBackend &) = delete;
    ComputeBackend &operator=(const ComputeBackend &) = delete;

    void computeBounds(std::vector<QuadTreeNode> &nodes);
    void build(std::vector<QuadTreeNode> &nodes);
    void calculateMass(std::vector<QuadTreeNode> &nodes);
    void computeForces(std::vector<QuadTreeNode> &nodes);
    void updatePositions(float dt);

  private:
    void checkError(cl_int status, const char *what);
    void checkBuildError(cl_int status, cl_program program,
                         cl_device_id device);
    cl_device_id findGpu();
    void ensureNodeBuffer(size_t nodeCount);
    bool contains(const QuadTreeNode *node, const Particle *particle);
    int childIndex(const QuadTreeNode *node, const Particle *particle);

    std::vector<Particle> &_particles;
    cl_context _context;
    cl_command_queue _queue;
    cl_program _program;
    cl_mem _particleBuffer;
    cl_mem _nodeBuffer;
    size_t _globalDimensions;
    size_t _nodeCapacity;
    cl_kernel _updatePositionsKernel;
    cl_kernel _computeForcesKernel;
};
