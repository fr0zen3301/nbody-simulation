#ifndef SIMULATION_HPP
#define SIMULATION_HPP

#include <vector>
#include "Body.hpp"
#include <SFML/Graphics.hpp>

class Simulation {
public:
    Simulation();

    void update(double dt);
    void draw(sf::RenderWindow& window); 

private:
    std::vector<Body> bodies;
};

#endif