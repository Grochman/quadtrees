﻿#include <iostream>
#include <math.h>
#include <chrono>
#include <algorithm>

#include <SFML/Graphics.hpp>

#include "QuadTreeNode.h"
#include "Particle.h"

void simulate(std::vector<Particle>& particles, QuadTreeNode& root, const double dt) {
    for (Particle& particle: particles) {
        Vector2d force = root.getTotalForce(&particle);
        particle.updateAcceletation(force);
    }
    for (Particle& particle : particles) {
        particle.move(dt);
    }
}

void updateVisualisation(std::vector<Particle>& particles, std::vector<sf::CircleShape>& particlesVisual, const sf::Vector2u windowDim, sf::RenderWindow& window) {
    for (int i = 0; i < particles.size(); i++) {
        particlesVisual[i].setPosition(sf::Vector2f(particles[i].position.x * windowDim.x, particles[i].position.y * windowDim.y));
        window.draw(particlesVisual[i]);
    }
}

void insertParticles(std::vector<Particle>& particles, QuadTreeNode& root) {
    root.reset();
    for (Particle& particle : particles) {
        root.addParticle(&particle);
    }
}

int main()
{
    srand(time(NULL));
    const unsigned int particleCount = 200;

    std::vector<Particle> particles;
    particles.reserve(particleCount);
    for (int i = 0; i < particleCount; i++) {
        particles.emplace_back();
    }
    
    QuadTreeNode root;

    const sf::Vector2u windowDimentions = { 600,600 };
    sf::RenderWindow window(sf::VideoMode(windowDimentions), "QuadTrees");
    
    std::vector<sf::CircleShape> particlesVisual;
    for (Particle& particle: particles) {
        sf::CircleShape circle(particle.mass * 5.f);
        circle.setPosition(sf::Vector2f(particle.position.x * windowDimentions.x, particle.position.y * windowDimentions.y));
        circle.setFillColor(sf::Color(255, 255, 255));
        particlesVisual.push_back(circle);
    }

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    while (window.isOpen()) {
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        double time_elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() / 1000000.f;
        std::cout << "fps: " << 1 / time_elapsed << '\n';
        begin = end;

        insertParticles(particles, root);
        simulate(particles, root, time_elapsed * 0.01);

        window.clear(sf::Color::Black);
        root.draw(window);
        updateVisualisation(particles, particlesVisual, windowDimentions, window);
        window.display();
    }

    return 0;
}
