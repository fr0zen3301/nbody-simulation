#include "Simulation.hpp"
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/Graphics/Color.hpp>

Simulation::Simulation() {
    // initial bodies
    bodies.emplace_back(Body(1e14, Vector2D(300, 300), Vector2D(0, 5)));
    bodies.emplace_back(Body(1e14, Vector2D(500, 300), Vector2D(0, -5)));

    // initialize trails
    trails.resize(bodies.size());
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

    // update bodies and record trails
    for (size_t i = 0; i < bodies.size(); ++i) {
        bodies[i].update(dt);

        // add position to trail
        sf::Vertex vertex;
        vertex.position = sf::Vector2f(static_cast<float>(bodies[i].position.x), static_cast<float>(bodies[i].position.y));
        vertex.color = bodies[i].color; 
        trails[i].push_back(vertex);

        // limit trail length
        if (trails[i].size() > 500) {
            trails[i].pop_front();
        }
    }
}

void Simulation::draw(sf::RenderWindow& window) {
    for (size_t i = 0; i < bodies.size(); ++i) {
        const Body& body = bodies[i];

        // draw body
        sf::CircleShape circle(5);
        circle.setPosition(sf::Vector2f(static_cast<float>(body.position.x), static_cast<float>(body.position.y)));
        circle.setFillColor(body.color); // apply the color
        circle.setOrigin(sf::Vector2f(5.f, 5.f));
        window.draw(circle);

        // Convert deque trail to sf::VertexArray for drawing
        sf::VertexArray trail(sf::PrimitiveType::LineStrip, trails[i].size());
        for (size_t j = 0; j < trails[i].size(); ++j) {
            trail[j] =trails[i][j];
        }
        window.draw(trail);
    }
}
