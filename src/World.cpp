#include "World.h"

#include <cstdlib>

World::World(unsigned int particleCount, ComputeBackendKind backendKind)
    : _particles(makeParticles(particleCount)),
      _backend(createComputeBackend(backendKind, _particles, _nodes)) {}

std::vector<Particle> World::makeParticles(unsigned int particleCount) {
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
