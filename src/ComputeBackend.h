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

    virtual void computeBounds(std::vector<QuadTreeNode> &nodes);
    virtual void build(std::vector<QuadTreeNode> &nodes);
    virtual void calculateMass(std::vector<QuadTreeNode> &nodes);
    virtual void computeForces(std::vector<QuadTreeNode> &nodes) = 0;
    virtual void updatePositions(float dt) = 0;

  protected:
    explicit ComputeBackend(std::vector<Particle> &particles);
    std::vector<Particle> &_particles;

  private:
    bool contains(const QuadTreeNode *node, const Particle *particle);
    int childIndex(const QuadTreeNode *node, const Particle *particle);
};

std::unique_ptr<ComputeBackend>
createComputeBackend(ComputeBackendKind kind,
                     std::vector<Particle> &particles);
