#include "Body.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Color.hpp>
#include <cmath>
#include <algorithm>

constexpr double MASS_RED = 8e6;
constexpr double MASS_ORANGE = 5e6;
constexpr double MASS_YELLOW = 2e6;
constexpr double MASS_CYAN = 1e5;
/*  
    Constructor gets mass, init position, init velocity.
    vel - initial velocity, can be set manually if the system is known (stable system),
    can be set to zero to simulate a body starting at rest.

    Velocity has 2 parameters:
    vx - velocity in x direction
    vy - velocity in y direction
    Vector2D vel(0.0, -50.0) ->  Upward, 50 units per second; Vector2D vel(50.0, 0.0) -> Right

    Initial velocity doesn't depend on position. A body can be put anywhere and given any velocity.
    The point is how it is applied over time.
*/
Body::Body(double m, const Vector2D& pos, const Vector2D& vel)  
    : mass(m), position(pos), velocity(vel), force(0, 0), acceleration(0, 0), color(getColorByMass(m)) {}
    

sf::Color Body::getColorByMass(double mass) {
    if (mass > MASS_RED) return sf::Color(157, 180, 255); // blue-white giant
    if (mass > MASS_ORANGE) return sf::Color::White;
    if (mass > MASS_YELLOW) return sf::Color(255, 240, 10); // yellow, sun-like
    if (mass > MASS_CYAN) return sf::Color(255, 150, 80); // orange dwarf
    return sf::Color(255, 100, 90); // red dwarf
}

float Body::getRadiusByMass(double mass) {
    float r = 2.f + 0.8f * static_cast<float>(std::log10(mass / 1e4));
    return std::clamp(r, 2.f, 10.f);
}

void Body::applyForce(const Vector2D& f) {
    force += f;
}

void Body::resetForce() {
    force = Vector2D(0, 0);
}

Vector2D Body::getPosition() const {
    return position;
}

Vector2D Body::computeGravitationalForce(const Body& other) const {
    Vector2D direction = other.position - position;
    double distance = direction.magnitude();

    double distSquared = distance * distance + SOFTENING * SOFTENING;

    if (distance < 1e-9) return Vector2D(0, 0); 

    Vector2D directionNorm = direction / distance;
    double forceMagnitude = (G * mass * other.mass) / distSquared;

    return directionNorm * forceMagnitude;
}
