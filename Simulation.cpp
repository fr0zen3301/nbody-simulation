#include "Simulation.hpp"
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/Graphics/Color.hpp>

Simulation::Simulation() {
    // initial bodies
    bodies.emplace_back(Body(1e14, Vector2D(300, 300), Vector2D(0, 5)));
    bodies.emplace_back(Body(1e14, Vector2D(500, 300), Vector2D(0, -5)));
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
        // draw trail
        const auto& trail = body.getTrail();
        sf::VertexArray line(sf::PrimitiveType::LineStrip, trail.size());
        for (size_t i = 0; i < trail.size(); ++i) {
            line[i].position = trail[i];
            line[i].color = sf::Color(255, 255, 255, static_cast<uint8_t>(255 * i / trail.size())); //fade effect
        }
        window.draw(line);
        // draw body
        sf::CircleShape circle(5.f);
        circle.setFillColor(sf::Color::White);
        Vector2D pos = body.getPosition();
        circle.setPosition(sf::Vector2f(static_cast<float>(pos.x), static_cast<float>(pos.y)));
        window.draw(circle);
    }
}
