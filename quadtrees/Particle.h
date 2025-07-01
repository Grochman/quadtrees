#pragma once

#include "Vector2d.h"
#include "QuadTreeNode.h"
#include <algorithm>


struct Particle {
    Vector2d position;
    Vector2d velocity;
    QuadTreeNode* quad = nullptr;
    inline static const double maxVelocity = 1.;

    Particle(const Vector2d& position) : position(position) {
        velocity = { double(rand()) / RAND_MAX - 0.5, double(rand()) / RAND_MAX - 0.5 };
    };

    void move(double dt) {
        position += {velocity.x* dt, velocity.y* dt};
        if (quad != nullptr) {
            quad->updateParticleOwnership(this);
        }
    }

    void updateVelocity(Vector2d velocity) {
        this->velocity += velocity;
        if (this->velocity.length() > maxVelocity){
            this->velocity.x = this->velocity.x * maxVelocity / this->velocity.length();
            this->velocity.y = this->velocity.y * maxVelocity / this->velocity.length();
        }
    }
};