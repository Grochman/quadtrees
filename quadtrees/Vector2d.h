#pragma once

struct alignas(8) float2 {
    float x = 0;
    float y = 0;
};

float length(const float2& vec);
float2 distance(const float2& vec1, const float2& vec2);
float2 unit(const float2& vector);
float2 sum(const float2& vec1, const float2& vec2);
float2 divide(const float2& vec, const float value);
