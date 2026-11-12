#pragma once

#include "ComputeBackend.h"
#include "Particle.h"
#include "QuadTreeNode.h"
#include <memory>
#include <vector>

class World {
  public:
    World(unsigned int particleCount, ComputeBackendKind backendKind);
    ~World() = default;

    std::vector<Particle> &getParticles() { return _particles; }
    std::vector<QuadTreeNode> &getNodes() { return _nodes; }

    void step(float dt) {
        _backend->computeBounds(_nodes);
        _backend->build(_nodes);
        _backend->calculateMass(_nodes);
        _backend->computeForces(_nodes);
        _backend->updatePositions(dt);
    }

  private:
    static std::vector<Particle> makeParticles(unsigned int particleCount);

    std::vector<Particle> _particles;
    std::vector<QuadTreeNode> _nodes;
    std::unique_ptr<ComputeBackend> _backend;
};
