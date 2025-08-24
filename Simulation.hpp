#ifndef SIMULATION_HPP
#define SIMULATION_HPP

#include <vector>
#include "Body.hpp"
#include <SFML/Graphics.hpp>
#include <deque>

class Simulation {
public:
    Simulation();

    void update(double dt);
    void draw(sf::RenderWindow& window); 
    void addBody(double mass, const Vector2D& position, const Vector2D& velocity);

private:
    std::vector<Body> bodies;
    std::vector<std::deque<sf::Vertex>> trails; 
};

#endif