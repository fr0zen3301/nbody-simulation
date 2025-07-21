#ifndef VECTOR2D_HPP
#define VECTOR2D_HPP

#include <cmath> 

struct Vector2D {
public:
    double x;
    double y;

    // Contructors
    Vector2D() : x(0), y(0) {}
    Vector2D(double x_val, double y_val) : x(x_val), y(y_val) {}

    // Operator overloads, no implementation of Vector2D.cpp is needed
    Vector2D operator+(const Vector2D& other) const {
        return Vector2D(x + other.x, y + other.y);
    }

    Vector2D operator-(const Vector2D& other) const {
        return Vector2D(x - other.x, y - other.y);
    }

    Vector2D operator*(double scalar) const {
        return Vector2D(x * scalar, y * scalar);
    }

    Vector2D operator/(double scalar) const {
        return Vector2D(x / scalar, y / scalar);
    }

    Vector2D& operator+=(const Vector2D& other) {
        x += other.x;
        y += other.y;
        return *this;
    }

    Vector2D& operator-=(const Vector2D& other) {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    // Magnitude and normalization
    double magnitude() const {
        return std::sqrt(x * x + y * y);
    }
    Vector2D normalized() const {
        double mag = magnitude();
        if (mag == 0) return Vector2D(0, 0);
        return Vector2D(x / mag, y / mag);
    }
};

#endif