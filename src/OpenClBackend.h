#pragma once

#include "ComputeBackend.h"
#include <CL/cl.h>

class OpenClBackend : public ComputeBackend {
  public:
    OpenClBackend(std::vector<Particle> &particles,
                  std::vector<QuadTreeNode> &nodes);
    ~OpenClBackend() override;

    void computeForces() override;
    void updatePositions(float dt) override;

  private:
    void checkError(cl_int status, const char *what);
    void checkBuildError(cl_int status, cl_program program,
                         cl_device_id device);
    cl_device_id findGpu();
    void ensureNodeBuffer(size_t nodeCount);

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
