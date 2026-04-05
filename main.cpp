#include <SFML/Graphics.hpp>
#include "Simulation.hpp"
#include <optional>
#include <functional>
#include <algorithm>
#include <cmath>
#include <random>

// High quality random number generation for star field. No hidden global state, distributions are explicit.
std::mt19937 rng(static_cast<unsigned int>(std::time(nullptr)));
std::uniform_real_distribution<float> dist(-2500.f, 2500.f);

int main() {
    sf::RenderWindow window(sf::VideoMode({800, 600}), "N-body Simulation");
    window.setFramerateLimit(60);
    // Generate star field
    std::vector<sf::CircleShape> stars;
    
    for (int i = 0; i < 1000; i++) {
        sf::CircleShape star(1.f);
        star.setFillColor(sf::Color(200, 200, 200));

        float x = dist(rng);
        float y = dist(rng);
        star.setPosition(sf::Vector2f(x, y));
        
        stars.push_back(star);
    }

    sf::Vector2f position(0.f, 0.f);
    sf::Vector2f size(800.f, 600.f);
    sf::Vector2f center(0.f, 0.f);
    sf::View view(sf::FloatRect(position, size));
    window.setView(view);

    float zoomVelocity = 0.f; // accumulated scroll input
    float currentZoom = 1.f;  // track zoom level

    Simulation sim;
    const float TIME_SCALE = 10.f;
    sf::Clock clock;
    bool mouseHeld = false;

    while (window.isOpen()) {
        // Handle events
        while (const std::optional<sf::Event> event = window.pollEvent()) {

            if (event->is<sf::Event::Closed>()) {
                window.close();
            }

            else if (const auto* wheel = event->getIf<sf::Event::MouseWheelScrolled>()) {
                if (wheel->wheel == sf::Mouse::Wheel::Vertical) {
                    zoomVelocity += wheel->delta;
                }
            }
        }
        // Apply zoom (Once per Frame)
        if (std::abs(zoomVelocity) > 0.01f) {
            sf::Vector2i mousePixel = sf::Mouse::getPosition(window);
            sf::Vector2f beforeZoomCoords = window.mapPixelToCoords(mousePixel);

            float zoomFactor = std::pow(0.9f, zoomVelocity);

            currentZoom *= zoomFactor;
            currentZoom = std::clamp(currentZoom, 0.5f, 10.f);

            view.zoom(zoomFactor);
            window.setView(view);

            sf::Vector2f afterZoomCoords = window.mapPixelToCoords(mousePixel);
            view.move(beforeZoomCoords - afterZoomCoords);
            window.setView(view);

            zoomVelocity *= 0.85f; // damping trackpad inertia killer 
        }
        //  Mouse spawning (frame-based, not event-based)
        if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
            if (!mouseHeld) {
                sf::Vector2i pixelPos = sf::Mouse::getPosition(window);
                sf::Vector2f worldPos = window.mapPixelToCoords(pixelPos);

                sim.addBody(
                        1e6,
                        Vector2D(worldPos.x, worldPos.y),
                        Vector2D(0.0, 0.0)
                    );

                mouseHeld = true;
            }
        } else {
            mouseHeld = false;   // reset on release to allow new spawns on next click
        }

        // Update simulation
        float dt = clock.restart().asSeconds();
        sim.update(dt * TIME_SCALE);

        // Render
        window.clear(sf::Color::Black);

        for (const auto& star : stars)
            window.draw(star);

        sim.draw(window);
        window.display();
    }

    return 0;
    }
