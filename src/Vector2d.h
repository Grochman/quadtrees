#pragma once

struct alignas(8) float2 {
    float x = 0;
    float y = 0;
};

static_assert(sizeof(float2) == 8, "float2 layout must match OpenCL float2");

inline float2 divide(float2 vec, float scalar) {
    return {vec.x / scalar, vec.y / scalar};
}
