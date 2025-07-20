#pragma once

#include "Vector2d.h"
#include "QuadTreeNode.h"
#include <algorithm>
#include <math.h>

struct Particle {
    Vector2d position;
    Vector2d acceleration;
    Vector2d velocity;
    QuadTreeNode* quad = nullptr;
    double mass = 1;

    Particle() {
        position = { double(rand()) / RAND_MAX, double(rand()) / RAND_MAX };
        velocity = { double(rand()) / RAND_MAX - 0.5, double(rand()) / RAND_MAX - 0.5 };
        mass = double(rand()) / RAND_MAX;
        acceleration = { 0,0 };
    };

    void move(double dt) {
        position += {velocity.x * dt, velocity.y * dt};
        velocity += {acceleration.x * dt, acceleration.y * dt};
        acceleration = { 0,0 };
    }

    void updateAcceletation(Vector2d force) {
        this->acceleration += {force.x / mass, force.y / mass};
    }
};