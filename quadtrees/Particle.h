#pragma once

#include "Vector2d.h"
#include "QuadTreeNode.h"
#include <algorithm>
#include <math.h>

struct QuadTreeNode;

struct Particle {
    Vector2d position;
    Vector2d velocity;
    Vector2d acceleration;
    float mass = 1;
    QuadTreeNode* quad = nullptr;
};


void initParticle(Particle* particle) {
    particle->position = { double(rand()) / RAND_MAX, double(rand()) / RAND_MAX };
    particle->velocity = { double(rand()) / RAND_MAX - 0.5, double(rand()) / RAND_MAX - 0.5 };
    particle->mass = float(rand()) / RAND_MAX;
    particle->acceleration = { 0,0 };
};

void move(Particle* particle, double dt) {
    particle->position.x += particle->velocity.x * dt;
    particle->position.y += particle->velocity.y * dt;

    particle->velocity.x += particle->acceleration.x * dt;
    particle->velocity.y += particle->acceleration.y * dt;

    particle->acceleration = { 0,0 };
}

void updateAcceletation(Particle* particle, Vector2d force) {
    particle->acceleration.x += force.x / particle->mass;
    particle->acceleration.y += force.y / particle->mass;
}