#pragma once

#include "Vector2d.h"


struct QuadTreeNode;

struct Particle {
    Vector2d position;
    Vector2d velocity;
    Vector2d acceleration;
    float mass = 1;
};

void initParticle(Particle* particle);
void move(Particle* particle, double dt);
void updateAcceletation(Particle* particle, Vector2d force);
