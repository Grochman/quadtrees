#include <iostream>
#include <SFML/Graphics.hpp>

#include "QuadTreeNode.h"
#include "Particle.h"

void simulate(std::vector<Particle>& particles) {
    for (Particle& particle : particles) {
        particle.move({ double(rand()) / RAND_MAX - 0.5, double(rand()) / RAND_MAX - 0.5});
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
    particles[0].move({ 1,1 });
    particles[1].move({ 1,1 });


    const sf::Vector2u windowDimentions = { 600,600 };
    sf::RenderWindow window(sf::VideoMode(windowDimentions), "QuadTrees");
    std::vector<sf::CircleShape> particles_visual;
    for (int i = 0; i < particles.size(); ++i) {
        sf::CircleShape particle(1.f);
        particle.setPosition(sf::Vector2f( particles[i].position.x * windowDimentions.x, particles[i].position.y * windowDimentions.y));
        particle.setFillColor(sf::Color(255, 255, 255));
        particles_visual.push_back(particle);
    }

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if ((event->is<sf::Event::KeyPressed>() &&
               event->getIf<sf::Event::KeyPressed>()->code == sf::Keyboard::Key::Escape)){ 
                window.close();
            }
        }

        simulate(particles);
        updateVisualisation(particles, particles_visual, windowDimentions);
        
        window.clear(sf::Color::Black);
        for (const auto& particle : particles_visual) {
            window.draw(particle);
        }
        window.display();
    }

    return 0;
}
