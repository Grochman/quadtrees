#pragma once

#include "ComputeBackend.h"

class OpenMpBackend : public ComputeBackend {
  public:
    OpenMpBackend(std::vector<Particle> &particles,
                  std::vector<QuadTreeNode> &nodes);

    void computeBounds() override;
    void computeForces() override;
    void updatePositions(float dt) override;

  private:
    float2 calculateForce(float2 distance, float mass,
                          const Particle &particle);
    float2 totalForce(int particleIdx);
};
