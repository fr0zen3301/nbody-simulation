#include <SFML/Graphics.hpp>
#include "imgui.h"
#include "imgui-SFML.h"
#include "Simulation.hpp"
#include <optional>
#include <functional>
#include <algorithm>
#include <cmath>
#include <random>
#include <sstream>
#include <iomanip>

// High quality random number generation for star field. No hidden global state, distributions are explicit.
std::mt19937 rng(static_cast<unsigned int>(std::time(nullptr)));
std::uniform_real_distribution<float> dist(-2500.f, 2500.f);

int main() {
    sf::RenderWindow window(sf::VideoMode({1280, 800}), "EMMA (Engine for Multi-body Mechanics and Astrophysics)");
    window.setFramerateLimit(60);

    if (!ImGui::SFML::Init(window)) {
        return -1; // ImGui-SFML failed to initialize
    }

    // ImGuin styling
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 6.f;
    style.FrameRounding = 4.f;
    style.Colors[ImGuiCol_WindowBg]      = ImVec4(0.05f, 0.05f, 0.08f, 0.85f);
    style.Colors[ImGuiCol_TitleBg]       = ImVec4(0.08f, 0.08f, 0.12f, 1.f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.10f, 0.10f, 0.16f, 1.f);
    style.Colors[ImGuiCol_FrameBg]       = ImVec4(0.12f, 0.12f, 0.18f, 1.f);
    style.Colors[ImGuiCol_SliderGrab]    = ImVec4(0.55f, 0.65f, 0.9f, 1.f);
    style.Colors[ImGuiCol_Button]        = ImVec4(0.15f, 0.18f, 0.28f, 1.f);


    // Custom fonts
    sf::Font font;
    if (!font.openFromFile("fonts/font.otf")) {
        return -1;
    }

    // Generate star field
    std::vector<sf::CircleShape> stars;
    std::uniform_real_distribution<float> radiusDist(0.5f, 1.5f);
    std::uniform_int_distribution<int> brightDist(120, 255);
    std::uniform_int_distribution<int> tintDust(0, 2);
    
    for (int i = 0; i < 1000; i++) {
        sf::CircleShape star(radiusDist(rng));

        int b = brightDist(rng);
        sf::Color c;
        switch (tintDust(rng)) {
            case 0: c = sf::Color(b, b, static_cast<std::uint8_t>(std::min(255, b + 30))); break;
            case 1: c = sf::Color(static_cast<std::uint8_t>(std::min(255, b + 30)), b, b); break;
            default: c = sf::Color(b, b, b); break;
        }
        star.setFillColor(c);

        star.setPosition(sf::Vector2f(dist(rng), dist(rng)));
        stars.push_back(star);
    }

    sf::Vector2f position(0.f, 0.f);
    sf::Vector2f size(1280.f, 800.f);
    sf::View view(sf::FloatRect(position, size));
    window.setView(view);

    // HUD view: 1 unit = 1 pixel, top-left at (0,0), unaffected by world zoom
    sf::View hudView(sf::FloatRect({0.f, 0.f}, {1280.f, 800.f}));

    // HUD text (SFML 3 requires the font in the constructor)
    sf::Text hudText(font);
    hudText.setCharacterSize(16);
    hudText.setFillColor(sf::Color::White);
    hudText.setPosition({10.f, 10.f});

    float zoomVelocity = 0.f; // accumulated scroll input
    float currentZoom = 1.f;  // track zoom level

    Simulation sim;
    const float TIME_SCALE = 10.f;
    const float FIXED_DT = 1.f / 120.f;
    float accumulator = 0.f;
    sf::Clock clock;

    // Spawn-panel state (driven by the ImGui sliders)
    float spawnMassExp = 6.f; // body mass = 10^spawnMassExp
    float spawnSpeed = 0.f;   // initial speed
    float spawnAngle = 0.f;   // initial direction, in radians

    while (window.isOpen()) {
        // Handle events
        while (const std::optional<sf::Event> event = window.pollEvent()) {
            ImGui::SFML::ProcessEvent(window, *event);

            if (event->is<sf::Event::Closed>()) {
                window.close();
            }

            else if (const auto* wheel = event->getIf<sf::Event::MouseWheelScrolled>()) {
                if (wheel->wheel == sf::Mouse::Wheel::Vertical) {
                    zoomVelocity += wheel->delta;
                }
            }
            else if (const auto* resized = event->getIf<sf::Event::Resized>()) {
                sf::Vector2f newSize(static_cast<float>(resized->size.x),
                                    static_cast<float>(resized->size.y));
                view.setSize(newSize * currentZoom); // keep current zoom level
                window.setView(view);

                // keep the HUD pinned to pixels at the new window size
                hudView.setSize(newSize);
                hudView.setCenter(newSize / 2.f);
            }
            else if (const auto* mouseButton = event->getIf<sf::Event::MouseButtonPressed>()) {
                // Only fires for clicks inside the content area
                if (mouseButton->button == sf::Mouse::Button::Left
                        && !ImGui::GetIO().WantCaptureMouse) {
                    sf::Vector2f worldPos = window.mapPixelToCoords(mouseButton->position);
                    sim.addBody(
                            1e6,
                            Vector2D(worldPos.x, worldPos.y),
                            Vector2D(0.0, 0.0)
                        );
                }
            }
        }

        // Per-frame timestep, shared by ImGui and the simulation
        sf::Time elapsed = clock.restart();
        float dt = elapsed.asSeconds();
        ImGui::SFML::Update(window, elapsed);

        // ImGui control panel
        ImGui::Begin("Spawn Controls");
        ImGui::SliderFloat("Mass (10^x)", &spawnMassExp, 4.f, 8.f);
        ImGui::SliderFloat("Speed", &spawnSpeed, 0.f, 50.f);
        ImGui::SliderAngle("Direction", &spawnAngle);
        if (ImGui::Button("Spawn at view center")) {
            sf::Vector2f c = view.getCenter();
            Vector2D vel(std::cos(spawnAngle) * spawnSpeed,
                         std::sin(spawnAngle) * spawnSpeed);
            sim.addBody(std::pow(10.f, spawnMassExp),
                        Vector2D(c.x, c.y), vel);
        }
        if (ImGui::Button("Load Solar System (similar system)")) {
            sim.loadSolarSystem();
        }
        if (ImGui::Button("Three Body System")) {
            sim.loadFigureEight();
        }
        ImGui::Text("Bodies: %zu", sim.getBodyCount());
        ImGui::End();

        // Apply zoom (Once per Frame)
        if (std::abs(zoomVelocity) > 0.01f) {
            sf::Vector2i mousePixel = sf::Mouse::getPosition(window);
            sf::Vector2f beforeZoomCoords = window.mapPixelToCoords(mousePixel);

            float rawFactor = std::pow(0.9f, zoomVelocity);
            float newZoom = std::clamp(currentZoom * rawFactor, 0.5f, 10.f);
            float effectiveFactor = newZoom / currentZoom; 
            currentZoom = newZoom;

            view.zoom(effectiveFactor);
            window.setView(view);

            sf::Vector2f afterZoomCoords = window.mapPixelToCoords(mousePixel);
            view.move(beforeZoomCoords - afterZoomCoords);
            window.setView(view);

            zoomVelocity *= 0.85f; // damping trackpad inertia killer 
        }

        // Update simulation (dt computed above)
        accumulator += dt * TIME_SCALE;
        while (accumulator >= FIXED_DT) {
            sim.update(FIXED_DT);
            accumulator -= FIXED_DT;
        }

        // Render
        window.clear(sf::Color::Black);

        for (const auto& star : stars)
            window.draw(star);

        sim.draw(window);

        // Draw HUD on top, in screen space
        float fps = (dt > 0.f) ? (1.f / dt) : 0.f;
        std::ostringstream hud;
        hud << "Bodies: " << sim.getBodyCount() << "\n"
            << "Energy: " << std::scientific << std::setprecision(2) << sim.totalEnergy() << "\n"
            << "FPS: " << std::fixed << std::setprecision(0) << fps;
        hudText.setString(hud.str());

        window.setView(hudView);
        window.draw(hudText);
        window.setView(view); // restore world view

        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();
    return 0;
}
