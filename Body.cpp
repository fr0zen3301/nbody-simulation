#include "Body.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Color.hpp>

Body::Body(double m, const Vector2D& pos, const Vector2D& vel) 
    : mass(m), position(pos), velocity(vel), force(0, 0) {

        // set color by its mass
        if (mass > 8e6)
            color = sf::Color::Red;
        else if (mass > 5e6)
            color = sf::Color(255, 140, 0); // orange
        else if (mass > 2e6)
            color = sf::Color::Yellow;
        else if (mass > 1e6)
            color = sf::Color::Cyan;
        else
            color = sf::Color::White;
    }
    
void Body::applyForce(const Vector2D& f) {
    force += f;
}

void Body::resetForce() {
    force = Vector2D(0, 0);
}

void Body::update(double dt) {
    // F = ma -> a = F / m
    Vector2D acceleration = force / mass;
    velocity += acceleration * dt;
    position += velocity * dt;

    // add to trail
    trail.push_back(sf::Vector2f(static_cast<float>(position.x), static_cast<float>(position.y)));
    if (trail.size() > maxTrailLength) {
        trail.erase(trail.begin()); // remove oldest points
    }
}

Vector2D Body::getPosition() const {
    return position;
}

Vector2D Body::computeGravitationalForce(const Body& other) const {
    Vector2D direction = other.position - position;
    double distance = direction.magnitude();

    //prevent division by 0 or very small distances
    if (distance < 1e-5) {
        return Vector2D(0, 0);
    }

    Vector2D directionNorm = direction / distance;

    // Newton's law of universal gravitation
    double forceMagnitude = (G * mass * other.mass) / (distance * distance);

    return directionNorm * forceMagnitude;
}
