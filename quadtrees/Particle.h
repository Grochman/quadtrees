#pragma once

#include "Vector2d.h"


struct QuadTreeNode;

struct Particle {
    float2 position;
    float2 velocity;
    float2 acceleration;
    float mass = 1.f;
};

void initParticle(Particle* particle);
void move(Particle* particle, double dt);
void updateAcceletation(Particle* particle, float2 force);
