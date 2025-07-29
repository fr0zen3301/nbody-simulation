#pragma once 
#include "Vector2D.hpp"
#include <vector>
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Color.hpp>

class Body {
    std::vector<sf::Vector2f> trail; // to store past positions
    const size_t maxTrailLength = 300; // can be tuned

public:
    Body(double mass, const Vector2D& position, const Vector2D& velocity);

    void applyForce(const Vector2D& force);
    void update(double dt);
    void resetForce();
    Vector2D getPosition() const;
    const std::vector<sf::Vector2f>& getTrail() const { return trail; }
    double mass;
    Vector2D position;
    Vector2D velocity;
    Vector2D force; // accumulated force
    sf::Color color;

    // Gravitational force between this and other body
    Vector2D computeGravitationalForce(const Body& other) const;

private:
    static constexpr double G = 6.674e-11; // Gravitational constant, calculated as needed
};
