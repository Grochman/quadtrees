#pragma once

struct Vector2d {
    double x = 0;
    double y = 0;
};

double length(const Vector2d& vec);
Vector2d distance(const Vector2d& vec1, const Vector2d& vec2);
Vector2d unit(const Vector2d& vector);
Vector2d sum(const Vector2d& vec1, const Vector2d& vec2);
Vector2d divide(const Vector2d& vec, const double value);
