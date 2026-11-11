#pragma once

#include "Vector2d.h"

struct Particle {
    float2 position;
    float2 velocity;
    float2 acceleration;
    float mass = 1.f;
    float _pad = 0.f;
};

static_assert(sizeof(Particle) == 32,
              "Particle layout must match the OpenCL kernel");
