#include "Simulation.hpp"

Simulation::Simulation() {
    // initial bodies
    bodies.emplace_back(Body(1e14, Vector2D(300, 300), Vector2D(0, 15)));
    bodies.emplace_back(Body(1e14, Vector2D(500, 300), Vector2D(0, -15)));
}

void Simulation::update(double dt) {
    // reset forces
    for (auto& body : bodies) {
        body.resetForce();
    }

    for (size_t i = 0; i < bodies.size(); ++i) {
        for (size_t j = i + 1; j < bodies.size(); ++j) {
            Vector2D force = bodies[i].computeGravitationalForce(bodies[j]);
            bodies[i].applyForce(force);
            bodies[j].applyForce(force * -1);
        }
    }

    // update bodies
    for (auto& body : bodies) {
        body.update(dt);
    }
}

void Simulation::draw(sf::RenderWindow& window) {
    for (const auto& body : bodies) {
        sf::CircleShape circle(5.f);
        circle.setFillColor(sf::Color::White);
        Vector2D pos = body.getPosition();
        circle.setPosition(sf::Vector2f(static_cast<float>(pos.x), static_cast<float>(pos.y)));
        window.draw(circle);
    }
}
