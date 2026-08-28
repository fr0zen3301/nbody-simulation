#include "Simulation.hpp"
#include "QuadTree.hpp"
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/Graphics/Color.hpp>
#include <cmath>
#include <algorithm>

Simulation::Simulation() {
    // hierarchical triple
    const Vector2D c(400, 300);
    const double m = 1e14;

    // binary, separation 80
    bodies.emplace_back(m, c + Vector2D(-40, 0), Vector2D(0.0, 6.46));
    bodies.emplace_back(m, c + Vector2D(40, 0), Vector2D(0.0, -6.46));

    // companion orbiting the pair
    bodies.emplace_back(5e6, c + Vector2D(0, -280), Vector2D(6.90, 0.0));

    // initialize trails
    trails.resize(bodies.size());
}

void Simulation::update(double dt) {
    // drift - moving positions using current velocity + stored acceleration
    for (auto& body : bodies) {
        body.position += body.velocity * dt + body.acceleration * (0.5 * dt * dt);
    }

    // recompute forces at the new position: Barnes-Hut - O(N log N)
    for (auto& body : bodies) body.resetForce();
    
    if (!bodies.empty()) {
        // bounding square around all bodies
        Vector2D lo = bodies[0].position, hi = bodies[0].position;
        for (const auto& b : bodies) {
            lo.x = std::min(lo.x, b.position.x);
            lo.y = std::min(lo.y, b.position.y);
            hi.x = std::max(hi.x, b.position.x);
            hi.y = std::max(hi.y, b.position.y);
        }
        Vector2D center((lo.x + hi.x) / 2, (lo.y + hi.y) / 2);
        double halfSize = std::max(hi.x - lo.x, hi.y - lo.y) / 2 + 1.0;

        QuadTree tree(center, halfSize);
        for (auto& b : bodies) tree.insert(&b);
        for (auto& b : bodies) b.applyForce(tree.computeForce(b, 0.5));
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
    for (size_t i = 0; i < bodies.size(); ++i) {
        for (size_t j = i + 1; j < bodies.size(); ++j) {
            double r = (bodies[j].position - bodies[i].position).magnitude();
            double mergeDistance = Body::getRadiusByMass(bodies[i].mass) + Body::getRadiusByMass(bodies[j].mass);
            if (r < mergeDistance) {
                double m1 = bodies[i].mass, m2 = bodies[j].mass;
                double total = m1 + m2;

                // conserve momentum and center of mass
                bodies[i].velocity = (bodies[i].velocity * m1 + bodies[j].velocity * m2) / total;
                bodies[i].position = (bodies[i].position * m1 + bodies[j].position * m2) / total;
                bodies[i].mass = total;
                bodies[i].color = Body::getColorByMass(total);
                bodies[i].acceleration = Vector2D(0, 0);

                // tiny bit of state
                flashes.push_back({bodies[i].position, 0.f});
                
                // remove the absorbed body
                bodies.erase(bodies.begin() + j);
                trails.erase(trails.begin() + j);
                --j;
            }
        }
    }
    constexpr float FLASH_LIFETIME = 0.5f;
    for (auto& f : flashes) f.age += static_cast<float>(dt);
    flashes.erase(std::remove_if(flashes.begin(), flashes.end(), [](const MergeFlash& f) { return f.age > 0.5f; }), flashes.end());
}

void Simulation::draw(sf::RenderWindow& window) {
    for (size_t i = 0; i < bodies.size(); ++i) {
        const Body& body = bodies[i];

        // draw body
        float radius = Body::getRadiusByMass(body.mass);
        sf::Vector2f pos(static_cast<float>(body.position.x),
                         static_cast<float>(body.position.y));

        // solid body core
        sf::CircleShape circle(radius);
        circle.setFillColor(body.color);
        circle.setOrigin(sf::Vector2f(radius, radius));
        circle.setPosition(pos);
        window.draw(circle);

        // trail with fade
        size_t n = trails[i].size();
        sf::VertexArray trail(sf::PrimitiveType::LineStrip, n);
        for (size_t j = 0; j < n; ++j) {
            trail[j] = trails[i][j];
            trail[j].color.a = static_cast<std::uint8_t>(255 * (j + 1) / n);
        }
        window.draw(trail);
    }
    
    // merge flashes -> expansion
    for (const auto& f : flashes) {
        float t = f.age / 0.5f;
        float r = 5.f + 30.f * t;
        sf::CircleShape ring(r);
        ring.setOrigin(sf::Vector2f(r, r));
        ring.setPosition(sf::Vector2f(static_cast<float>(f.position.x),
                                      static_cast<float>(f.position.y)));
        ring.setFillColor(sf::Color::Transparent);
        ring.setOutlineThickness(2.f);
        ring.setOutlineColor(sf::Color(255, 255, 255,
                             static_cast<std::uint8_t>(255 * (1.f - t))));
        window.draw(ring);
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
    constexpr double softening = 5.0;
    for (size_t i = 0; i < bodies.size(); ++i) {
        for (size_t j = i + 1; j < bodies.size(); ++j) {
            double r = (bodies[j].position - bodies[i].position).magnitude();
            potential += -G * bodies[i].mass * bodies[j].mass / std::sqrt(r * r + Body::SOFTENING * Body::SOFTENING);
        }
    }
    return kinetic + potential;
}

void Simulation::clear() {
    bodies.clear();
    trails.clear();
    flashes.clear();
}

void Simulation::loadSolarSystem() {
    clear();

    constexpr double G = 6.674e-11;
    const Vector2D sunPos(400, 300);
    const double sunMass = 1e14;
    
    // place a planet on a circular orbit at radius r
    auto addPlanet = [&](double mass, double r, double angleDeg) {
        double a = angleDeg * M_PI / 180.0;
        Vector2D pos(sunPos.x + std::cos(a) * r,
                     sunPos.y + std::sin(a) * r);
        double v = std::sqrt(G * sunMass / r);

        Vector2D vel(-std::sin(a) * v, std::cos(a) * v);
        bodies.emplace_back(mass, pos, vel);
        trails.emplace_back();
    };

    // sun
    bodies.emplace_back(sunMass, sunPos, Vector2D(0, 0));
    trails.emplace_back();

    // addPlanet: -- mass -- r -- start angle
    addPlanet(1e5, 90, 0);    // how inner dwarf
    addPlanet(4e5, 140, 130);
    addPlanet(1e6, 200, 250);
    addPlanet(3e6, 280, 60);
    addPlanet(5e6, 380, 180); // outer dwarf

    Vector2D totalP(0, 0);
    for (size_t k = 1; k < bodies.size(); ++k)
        totalP += bodies[k].velocity * bodies[k].mass;
    bodies[0].velocity = totalP / (-sunMass);
}

void Simulation::loadFigureEight() {
    clear();

    // Chenciner-Montgomery figure-eight
    const double M = 1e14; // all three bodies
    const double L = 200.0;
    const Vector2D c(400, 300);

    const double V = std::sqrt(Body::G * M / L);

    Vector2D r1(0.97000436, -0.24308753);
    Vector2D v3(-0.93240737, -0.86473146);
    Vector2D v1 = v3 * -0.5;

    bodies.emplace_back(M, c + r1 * L, v1 * V);
    bodies.emplace_back(M, c - r1 * L, v1 * V);
    bodies.emplace_back(M, c, v3 * V);
    for (int k = 0; k < 3; ++k) trails.emplace_back();
}