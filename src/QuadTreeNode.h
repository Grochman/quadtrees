#pragma once

#include "Vector2d.h"

struct QuadTreeNode {
    float2 position = {0, 0};
    float2 dimensions = {1, 1};
    float2 massCenter = {0, 0};
    float mass = 0;
    int leavesIdx = -1;
    int particleIdx = -1;
    int _pad = 0;
};

static_assert(sizeof(QuadTreeNode) == 40,
              "QuadTreeNode layout must match the OpenCL kernel");
