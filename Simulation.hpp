#ifndef SIMULATION_HPP
#define SIMULATION_HPP

#include <vector>
#include "Body.hpp"
#include <SFML/Graphics.hpp>
#include <deque>

class Simulation {
public:
    double totalEnergy() const;
    Simulation();

    void update(double dt);
    void draw(sf::RenderWindow& window); 
    void addBody(double mass, const Vector2D& position, const Vector2D& velocity);
    void clear();
    void loadSolarSystem();
    size_t getBodyCount() const { return bodies.size(); }

    struct MergeFlash {
        Vector2D position;
        float age = 0.f;
    };
    std::vector<MergeFlash> flashes;

private:
    std::vector<Body> bodies;
    std::vector<std::deque<sf::Vertex>> trails; 
};

#endif