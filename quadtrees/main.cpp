#include <iostream>
#include <chrono>

#include <SFML/Graphics.hpp>

#include "QuadTree.h"
#include "Particle.h"


int main()
{
    srand(time(NULL));
    const unsigned int particleCount = 5;

    std::vector<Particle> particles;
    particles.reserve(particleCount);
    for (int i = 0; i < particleCount; i++) {
        particles.emplace_back();
        initParticle(&particles[i]);
    }
    
    QuadTree tree;
    
    const sf::Vector2u windowDimentions = { 600,600 };
    sf::RenderWindow window(sf::VideoMode(windowDimentions), "QuadTrees");

    Vector2d scale = { window.getSize().x / 1., window.getSize().y / 1. };
    Vector2d translation = { 0. * scale.x , 0. * scale.y };
    double particleScale = 1;

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    while (window.isOpen()) {
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        double timeElapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() / 1000000.f;
        std::cout << "fps: " << 1 / timeElapsed << '\n';
        begin = end;

        tree.reset();
        tree.insert(particles);
        tree.calculateMass();

        for (Particle& particle : particles) {
            Vector2d force = tree.getTotalForce(particle);
            updateAcceletation(&particle, force);
        }
        for (Particle& particle : particles) {
            move(&particle, timeElapsed* 0.07);
        }

        window.clear(sf::Color::Black);
        tree.draw(window, scale, translation);
        for (Particle& particle : particles) {
            double radius = std::max(particle.mass * 5.f * particleScale, 1.);
            sf::CircleShape c(radius);
            c.setPosition(sf::Vector2f(particle.position.x * scale.x - radius + translation.x, particle.position.y * scale.y - radius + translation.y));
            c.setFillColor(sf::Color::White);
            window.draw(c);
        }
        window.display();
    }

    return 0;
}
