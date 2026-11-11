#pragma once

#include "Particle.h"
#include "QuadTree.h"
#include <CL/cl.h>
#include <vector>

class ComputeBackend {
  public:
    ComputeBackend(std::vector<Particle> &particles);
    ~ComputeBackend();

    void computeForces(std::vector<QuadTreeNode> &nodes);
    void updatePositions(float dt);

  private:
    void checkError(cl_int status, const char *what);
    void checkBuildError(cl_int status, cl_program program,
                         cl_device_id device);
    cl_kernel buildKernel(cl_device_id device, const char *source,
                          const char *functionName);

    std::vector<Particle> &_particles;
    cl_context _context;
    cl_command_queue _queue;
    cl_mem _particleBuffer;
    cl_mem _nodeBuffer;
    size_t _globalDimensions;
    cl_kernel _updatePositionsKernel;
    cl_kernel _computeForcesKernel;
};
