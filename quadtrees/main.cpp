#include <iostream>
#include <math.h>
#include <chrono>
#include <algorithm>

#include <SFML/Graphics.hpp>

#include "QuadTreeNode.h"
#include "Particle.h"

#define MIN 0.1

void simulate(std::vector<Particle>& particles, QuadTreeNode& root, double dt) {
    const double scale = 0.05;
    for (int i = 0; i < particles.size(); i++) {
        Particle& particle = particles[i];
        auto others = root.query(particle.position, 0.01);
        Vector2d totalForce= { 0,0 };
        for (Particle* other : others) {
            if (other != &particle) {
                Vector2d distance = { other->position.x - particle.position.x, other->position.y - particle.position.y };
                const double force = scale * (1 * 1) / pow(std::max(distance.length(), MIN), 2); //minimal distance
                totalForce.x += force * distance.x / distance.length();
                totalForce.y += force * distance.y / distance.length();
            }
        }
        Vector2d dv = { totalForce.x * dt, totalForce.y * dt };
        particle.updateVelocity(dv);
    }

    for (int i = 0; i < particles.size(); i++) {
        particles[i].move(dt);
    }
}

void updateVisualisation(std::vector<Particle>& particles, std::vector<sf::CircleShape>& particles_visual, const sf::Vector2u windowDim) {
    for (int i = 0; i < particles.size(); i++) {
        particles_visual[i].setPosition(sf::Vector2f(particles[i].position.x * windowDim.x, particles[i].position.y * windowDim.y));
    }
}

int main()
{
    srand(time(NULL));
    const unsigned int particleCount = 1000;

    std::vector<Particle> particles;
    particles.reserve(particleCount);
    for (int i = 0; i < particleCount; i++) {
        particles.emplace_back(Vector2d(double(rand()) / RAND_MAX, double(rand()) / RAND_MAX));
    }
    QuadTreeNode root;
    for (Particle& particle : particles) {
        root.addParticle(&particle);
    }

    const sf::Vector2u windowDimentions = { 600,600 };
    sf::RenderWindow window(sf::VideoMode(windowDimentions), "QuadTrees");
    std::vector<sf::CircleShape> particles_visual;
    for (int i = 0; i < particles.size(); ++i) {
        sf::CircleShape particle(3.f);
        particle.setPosition(sf::Vector2f( particles[i].position.x * windowDimentions.x, particles[i].position.y * windowDimentions.y));
        particle.setFillColor(sf::Color(255, 255, 255));
        particles_visual.push_back(particle);
    }

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if ((event->is<sf::Event::KeyPressed>() &&
               event->getIf<sf::Event::KeyPressed>()->code == sf::Keyboard::Key::Escape)){ 
                window.close();
            }
        }
        window.clear(sf::Color::Black);
        root.draw(window);
        for (const auto& particle : particles_visual) {
            window.draw(particle);
        }
        window.display();
        
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        double time_elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() / 1000000.f;
        std::cout << "fps: " << 1 / time_elapsed << '\n';
        std::cout << "time per frame : " << time_elapsed << '\n';

        std::chrono::steady_clock::time_point sim_start = std::chrono::steady_clock::now();
        simulate(particles, root, time_elapsed);
        std::chrono::steady_clock::time_point sim_end = std::chrono::steady_clock::now();
        double time_elapsed_on_simulation = std::chrono::duration_cast<std::chrono::microseconds>(sim_end - sim_start).count() / 1000000.f;
        std::cout << "simulation time: " << time_elapsed_on_simulation << '\n';


        begin = end;
        updateVisualisation(particles, particles_visual, windowDimentions);
    }

    return 0;
}
