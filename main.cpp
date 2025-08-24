#include <SFML/Graphics.hpp>
#include "Simulation.hpp"

int main() {
    sf::RenderWindow window(sf::VideoMode({800, 600}), "N-body Simulation");
    window.setFramerateLimit(60);

    Simulation sim;
    const float TIME_SCALE = 10.f;
    sf::Clock clock;
    bool mouseHeld = false;

    while(window.isOpen()) {
        while ( const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
        }
        // left click spawn
        // mass, velocity, and position are set by default values
        // TO DO: make a window to set those parameters before spawning
        if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
            if (!mouseHeld) {
                sf::Vector2i pixelPos = sf::Mouse::getPosition(window);
                sf::Vector2f worldPos = window.mapPixelToCoords(pixelPos);

                double mass = 1e6; // by default
                Vector2D velocity(0.0, 0.0);
                Vector2D position(worldPos.x, worldPos.y);

                sim.addBody(mass, position, velocity);
                mouseHeld = true;
            }
        } else {
            mouseHeld = false;
        }
    

        float dt = clock.restart().asSeconds();

        sim.update(dt * TIME_SCALE); // accelerating time to make it visible for a human eye

        window.clear(sf::Color::Black);
        sim.draw(window);
        window.display();
    }

    return 0;
}