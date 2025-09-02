#include "Particle.h"

#include <algorithm>
#include <math.h>


void initParticle(Particle* particle) {
    particle->position = { float(rand()) / RAND_MAX, float(rand()) / RAND_MAX };
    particle->velocity = { float(rand()) / RAND_MAX - 0.5f, float(rand()) / RAND_MAX - 0.5f };
    particle->mass = float(rand()) / RAND_MAX;
    particle->acceleration = { .0f,.0f };
};

void move(Particle* particle, double dt) {
    particle->velocity.x += particle->acceleration.x * dt/2;
    particle->velocity.y += particle->acceleration.y * dt/2;
    
    particle->position.x += particle->velocity.x * dt;
    particle->position.y += particle->velocity.y * dt;

    particle->velocity.x += particle->acceleration.x * dt/2;
    particle->velocity.y += particle->acceleration.y * dt/2;

    particle->acceleration = { 0,0 };
};

void updateAcceletation(Particle* particle, float2 force) {
    particle->acceleration.x += force.x / particle->mass;
    particle->acceleration.y += force.y / particle->mass;
};
