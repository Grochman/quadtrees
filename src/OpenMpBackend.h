#pragma once

#include "ComputeBackend.h"

class OpenMpBackend : public ComputeBackend {
  public:
    explicit OpenMpBackend(std::vector<Particle> &particles);

    void computeBounds(std::vector<QuadTreeNode> &nodes) override;
    void computeForces(std::vector<QuadTreeNode> &nodes) override;
    void updatePositions(float dt) override;

  private:
    float2 calculateForce(float2 distance, float mass,
                          const Particle &particle);
    float2 totalForce(int particleIdx, const std::vector<QuadTreeNode> &nodes);
};
