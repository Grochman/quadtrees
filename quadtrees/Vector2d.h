#pragma once

struct Vector2d {
    union {
        double x;
        double w;
    };
    union {
        double y;
        double h;
    };
    
    Vector2d() {};
    Vector2d(double x, double y) : x(x), y(y) {};

    void operator+=(const Vector2d& other) {
        x += other.x;
        y += other.y;
    }

    Vector2d operator+(const Vector2d& other) {
        return Vector2d(x += other.x, y += other.y);
    }

    Vector2d operator/(const int value) {
        return Vector2d(x / 2, y / 2);
    }
};