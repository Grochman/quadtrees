#include "Particle.h"
#include "Fail.h"

#include <cmath>
#include <cstdlib>

void initParticle(Particle *particle) {
    particle->position = {float(rand()) / float(RAND_MAX),
                          float(rand()) / float(RAND_MAX)};
    particle->velocity = {float(rand()) / float(RAND_MAX) - 0.5f,
                          float(rand()) / float(RAND_MAX) - 0.5f};
    particle->mass = (float(rand()) + 1.f) / (float(RAND_MAX) + 1.f);
    particle->acceleration = {.0f, .0f};
    particle->_pad = 0.f;
}

void move(Particle *particle, double dt) {
    if (!std::isfinite(dt) || dt < 0.0) {
        fatal("non-finite or negative integration step");
    }
    particle->velocity.x += particle->acceleration.x * dt / 2;
    particle->velocity.y += particle->acceleration.y * dt / 2;

    particle->position.x += particle->velocity.x * dt;
    particle->position.y += particle->velocity.y * dt;

    particle->velocity.x += particle->acceleration.x * dt / 2;
    particle->velocity.y += particle->acceleration.y * dt / 2;

    particle->acceleration = {0, 0};
}

void updateAcceletation(Particle *particle, float2 force) {
    if (!(particle->mass > 0.f) || !std::isfinite(particle->mass)) {
        fatal("particle mass must be finite and greater than zero");
    }
    particle->acceleration.x += force.x / particle->mass;
    particle->acceleration.y += force.y / particle->mass;
}
