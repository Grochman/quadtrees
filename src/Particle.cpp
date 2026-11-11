#include "Particle.h"
#include "Fail.h"

#include <cmath>

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
