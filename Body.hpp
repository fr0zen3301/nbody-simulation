#pragma once 
#include "Vector2D.hpp"

class Body {
public:
    Body(double mass, const Vector2D& position, const Vector2D& velocity);

    void applyForce(const Vector2D& force);
    void update(double dt);
    void resetForce();
    Vector2D getPosition() const;

    // Gravitational force between this and other body
    Vector2D computeGravitationalForce(const Body& other) const;

private:
    double mass;
    Vector2D position;
    Vector2D velocity;
    Vector2D force; // accumulated force

    static constexpr double G = 6.674e-11; // Gravitational constant, calculated as needed
};
