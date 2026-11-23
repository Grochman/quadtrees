#pragma once

#include "ComputeBackend.h"

class OpenMpBackend : public ComputeBackend {
  public:
    OpenMpBackend(std::vector<Particle> &particles,
                  std::vector<QuadTreeNode> &nodes);

    void computeBounds() override;
    void build() override;
    void calculateMass() override;
    void computeForces() override;
    void updatePositions(float dt) override;

  private:
    float2 calculateForce(float2 distance, float mass,
                          const Particle &particle);
    float2 totalForce(int particleIdx);
    void accumulateMass(int nodeIdx, int depth);
    int allocChildren();
    void initChildBoxes(int parentIdx, int firstChild);
    void partitionRange(int nodeIdx, int begin, int end, int splits[5]);
    void buildSubtree(int nodeIdx, int begin, int end);

    std::vector<int> _order;
    std::vector<int> _orderScratch;
    int _nextNode = 0;
};
