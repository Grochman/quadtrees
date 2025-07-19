#include <iostream>
#include <math.h>
#include <chrono>
#include <algorithm>

#include <SFML/Graphics.hpp>

#include "QuadTreeNode.h"
#include "Particle.h"

#define MIN 0.1

void simulate(std::vector<Particle>& particles, QuadTreeNode& root, double dt) {
    double scale = 1/pow(particles.size(),2);
    scale = 0.1;
    dt *= scale;
    for (int i = 0; i < particles.size(); i++) {
        Particle& particle = particles[i];
        auto others = root.query(particle.position, 2);
        Vector2d total_acceleration = { 0,0 };
        for (Particle* other : others) {
            if (other != &particle) {
                Vector2d distance = particle.position.distance(other->position);
                Vector2d unit_vector = distance.unit();
                double distance_length = distance.length();
                distance_length = std::min(distance_length, MIN);
                total_acceleration += { (1 / pow(distance_length, 2)) * unit_vector.x, (1 / pow(distance_length, 2)) * unit_vector.y };
            }
        }
        particle.updateAcceletation(total_acceleration);
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
    const unsigned int particleCount = 50;

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
        sf::CircleShape particle(5.f);
        particle.setPosition(sf::Vector2f( particles[i].position.x * windowDimentions.x, particles[i].position.y * windowDimentions.y));
        particle.setFillColor(sf::Color(255, 255, 255));
        particles_visual.push_back(particle);
    }

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    while (window.isOpen()) {
        window.clear(sf::Color::Black);
        root.draw(window);
        updateVisualisation(particles, particles_visual, windowDimentions);
        for (const auto& particle : particles_visual) {
            window.draw(particle);
        }
        window.display();


        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        double time_elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() / 1000000.f;
        std::cout << "fps: " << 1 / time_elapsed << '\n';
        begin = end;

        simulate(particles, root, time_elapsed);
    }

    return 0;
}
