#pragma once

#include "Particle.h"
#include "QuadTreeNode.h"
#include <memory>
#include <vector>

enum class ComputeBackendKind { OpenCL, OpenMP };

class ComputeBackend {
  public:
    virtual ~ComputeBackend() = default;
    ComputeBackend(const ComputeBackend &) = delete;
    ComputeBackend &operator=(const ComputeBackend &) = delete;

    virtual void computeBounds();
    virtual void build();
    virtual void calculateMass();
    virtual void computeForces() = 0;
    virtual void updatePositions(float dt) = 0;

  protected:
    ComputeBackend(std::vector<Particle> &particles,
                   std::vector<QuadTreeNode> &nodes);
    std::vector<Particle> &_particles;
    std::vector<QuadTreeNode> &_nodes;

  private:
    int childIndex(const QuadTreeNode *node, const Particle *particle);
};

std::unique_ptr<ComputeBackend>
createComputeBackend(ComputeBackendKind kind,
                     std::vector<Particle> &particles,
                     std::vector<QuadTreeNode> &nodes);
