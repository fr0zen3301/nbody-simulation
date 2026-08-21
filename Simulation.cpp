#include "Simulation.hpp"
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/Graphics/Color.hpp>

Simulation::Simulation() {
    // initial bodies
    bodies.emplace_back(Body(1e14, Vector2D(300, 300), Vector2D(0, 5)));
    bodies.emplace_back(Body(1e14, Vector2D(500, 300), Vector2D(0, -5)));
    bodies.emplace_back(Body(2e6, Vector2D(400, 200), Vector2D(-5, 0)));

    // initialize trails
    trails.resize(bodies.size());
}

void Simulation::update(double dt) {
    // drift - moving positions using current velocity + stored acceleration
    for (auto& body : bodies) {
        body.position += body.velocity * dt + body.acceleration * (0.5 * dt * dt);
    }

    // recompute forces at the new position
    for (auto& body : bodies) body.resetForce();
    for (size_t i = 0; i < bodies.size(); ++i) {
        for (size_t j = i + 1; j < bodies.size(); ++j) {
            Vector2D force = bodies[i].computeGravitationalForce(bodies[j]);
            bodies[i].applyForce(force);
            bodies[j].applyForce(force * -1);
        }
    }

    // kick - update velocity using the avg of old and new acceleration
    for (size_t i = 0; i < bodies.size(); ++i) {
        Vector2D newAccel = bodies[i].force / bodies[i].mass;
        bodies[i].velocity += (bodies[i].acceleration + newAccel) * (0.5 * dt);
        bodies[i].acceleration = newAccel; // for the next frame

        // record trail
        sf::Vertex vertex;
        vertex.position = sf::Vector2f(static_cast<float>(bodies[i].position.x),
                                        static_cast<float>(bodies[i].position.y));
        vertex.color = bodies[i].color;
        trails[i].push_back(vertex);
        if (trails[i].size() > 500) trails[i].pop_front();
    }

    // Merge bodies that get too close
    constexpr double mergeDistance = 8.0;
    for (size_t i = 0; i < bodies.size(); ++i) {
        for (size_t j = i + 1; j < bodies.size(); ++j) {
            double r = (bodies[j].position - bodies[i].position).magnitude();
            if (r < mergeDistance) {
                double m1 = bodies[i].mass, m2 = bodies[j].mass;
                double total = m1 + m2;

                // conserve momentum and center of mass
                bodies[i].velocity = (bodies[i].velocity * m1 + bodies[j].velocity * m2) / total;
                bodies[i].position = (bodies[i].position * m1 + bodies[j].position * m2) / total;
                bodies[i].mass = total;
                bodies[i].color = Body::getColorByMass(total);
                
                // remove the absorbed body
                bodies.erase(bodies.begin() + j);
                trails.erase(trails.begin() + j);
                --j;
            }
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

// add a body by a mouse click, parameters: mass, position, velocity 
void Simulation::addBody(double mass, const Vector2D& position, const Vector2D& velocity) {
    bodies.emplace_back(Body(mass, position, velocity));
    trails.emplace_back();
}

double Simulation::totalEnergy() const {
    constexpr double G = 6.674e-11;
    double kinetic = 0.0, potential = 0.0;

    for (const auto& b : bodies) {
        double v2 = b.velocity.x * b.velocity.x + b.velocity.y * b.velocity.y;
        kinetic += 0.5 * b.mass * v2;
    }
    for (size_t i = 0; i < bodies.size(); ++i) {
        for (size_t j = 0; j < bodies.size(); ++j) {
            double r = (bodies[j].position - bodies[i].position).magnitude();
            if (r > 1e-9)
                potential += -G * bodies[i].mass * bodies[j].mass / r;
        }
    }
    return kinetic + potential;
}