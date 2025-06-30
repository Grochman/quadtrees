#pragma once

#include "Vector2d.h"
#include "QuadTreeNode.h"

struct Particle {
    Vector2d position;
    QuadTreeNode* quad = nullptr;

    Particle(const Vector2d& position) : position(position) {};

    void move(const Vector2d& displacement) {
        position += displacement;
        quad->updateParticleOwnership(this);
    }
};