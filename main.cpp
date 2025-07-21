#include <SFML/Graphics.hpp>
#include "Simulation.hpp"

int main() {
    sf::RenderWindow window(sf::VideoMode({800, 600}), "N-body Simulation");
    window.setFramerateLimit(60);

    Simulation sim;

    sf::Clock clock;

    while(window.isOpen()) {
        while ( const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
        }

        float dt = clock.restart().asSeconds();

        sim.update(dt);

        window.clear(sf::Color::Black);
        sim.draw(window);
        window.display();
    }

    return 0;
}