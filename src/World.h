#pragma once

#include "ComputeBackend.h"
#include "Particle.h"
#include "QuadTree.h"
#include <vector>

class World {
  public:
    World(unsigned int particleCount)
        : _particles(makeParticles(particleCount)), _tree(_particles),
          _backend(_particles) {}
    ~World() = default;

    std::vector<Particle> &getParticles() { return _particles; }
    std::vector<QuadTreeNode> &getNodes() { return _tree.getNodes(); }

    void step(float dt) {
        _tree.computeBounds(); // reduction
        _tree.build();         // dick knows
        _tree.calculateMass(); // reduction?
        // Apprioximately sort the bodies by spatial distnce missing
        _backend.computeForces(_tree.getNodes());
        _backend.updatePositions(dt);
    }

  private:
    std::vector<Particle> makeParticles(unsigned int particleCount) {
        std::vector<Particle> particles;
        particles.reserve(particleCount);
        for (int i = 0; i < particleCount; i++) {
            particles.emplace_back();
            particles[i].position = {float(rand()) / float(RAND_MAX),
                                     float(rand()) / float(RAND_MAX)};
            particles[i].velocity = {float(rand()) / float(RAND_MAX) - 0.5f,
                                     float(rand()) / float(RAND_MAX) - 0.5f};
            particles[i].mass = (float(rand()) + 1.f) / (float(RAND_MAX) + 1.f);
            particles[i].acceleration = {.0f, .0f};
            particles[i]._pad = 0.f;
        }
        return particles;
    }
    std::vector<Particle> _particles;
    QuadTree _tree;
    ComputeBackend _backend;
};