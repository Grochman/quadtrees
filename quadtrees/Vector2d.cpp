#include "Vector2d.h"
#include "Fail.h"

#include <cmath>

float length(const float2 &vec) { return sqrt(pow(vec.x, 2) + pow(vec.y, 2)); }

float2 distance(const float2 &vec1, const float2 &vec2) {
    return float2{vec2.x - vec1.x, vec2.y - vec1.y};
}

float2 unit(const float2 &vector) {
    float l = length(vector);
    if (!(l > 0.f) || !std::isfinite(l)) {
        fatal("cannot normalize a zero or non-finite vector");
    }
    return float2{vector.x / l, vector.y / l};
}

float2 sum(const float2 &vec1, const float2 &vec2) {
    return float2{vec1.x + vec2.x, vec1.y + vec2.y};
}

float2 divide(const float2 &vec, const float value) {
    if (value == 0.f || !std::isfinite(value)) {
        fatal("division by zero or non-finite scalar");
    }
    return float2{vec.x / value, vec.y / value};
}
