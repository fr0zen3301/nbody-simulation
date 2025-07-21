#include <SFML/Graphics.hpp>
#include "Simulation.hpp"

int main() {
    sf::RenderWindow window(sf::VideoMode({800, 600}), "N-body Simulation");
    window.setFramerateLimit(60);

    Simulation sim;
    const float TIME_SCALE = 10.f;
    sf::Clock clock;

    while(window.isOpen()) {
        while ( const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
        }

        float dt = clock.restart().asSeconds();

        sim.update(dt * TIME_SCALE); // accelerating time to make it visible for a human eye

        window.clear(sf::Color::Black);
        sim.draw(window);
        window.display();
    }

    return 0;
}