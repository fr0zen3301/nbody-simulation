#pragma once 
#include "Vector2D.hpp"
#include <vector>
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Color.hpp>

class Body {
public:
    Body(double mass, const Vector2D& position, const Vector2D& velocity);

    void applyForce(const Vector2D& force);
    // void update(double dt);
    void resetForce();
    Vector2D getPosition() const;
    double mass;
    Vector2D position;
    Vector2D velocity;
    Vector2D force; // accumulated force
    Vector2D acceleration; // store acceleration between steps for Verlet
    sf::Color color;

    // Gravitational force between this and other body
    Vector2D computeGravitationalForce(const Body& other) const;
    static sf::Color getColorByMass(double mass);
    static float getRadiusByMass(double mass);

private:
    static constexpr double G = 6.674e-11; // Gravitational constant, calculated as needed
};
