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
    inline static double max_velocity = 1.5;


    Particle(const Vector2d& position) : position(position) {
        velocity = { double(rand()) / RAND_MAX - 0.5, double(rand()) / RAND_MAX - 0.5 };
        acceleration = { 0,0 };
    };

    void move(double dt) {
        position += {velocity.x * dt, velocity.y * dt};
        velocity += {acceleration.x * dt, acceleration.y * dt};
        if (velocity.length() > max_velocity) {
            double k = max_velocity / velocity.length();
            velocity.x *= k;
            velocity.y *= k;
        }
        acceleration = { 0,0 };
        if (quad != nullptr) {
            quad->updateParticleOwnership(this);
        }
    }

    void updateAcceletation(Vector2d acceleration) {
        this->acceleration += acceleration;
    }
};