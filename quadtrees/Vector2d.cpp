#include "Vector2d.h"

#include <cmath>


double length(const Vector2d& vec) {
    return sqrt(pow(vec.x, 2) + pow(vec.y, 2));
}

Vector2d distance(const Vector2d& vec1, const Vector2d& vec2) {
    return  Vector2d{ vec2.x - vec1.x, vec2.y - vec1.y };
}

Vector2d unit(const Vector2d& vector) {
    double l = length(vector);
    return Vector2d{ vector.x / l, vector.y / l };
}

Vector2d sum(const Vector2d& vec1, const Vector2d& vec2) {
    return Vector2d{ vec1.x + vec2.x, vec1.y + vec2.y };
}

Vector2d divide(const Vector2d& vec, const int value) {
    return Vector2d{ vec.x / value, vec.y / value };
}