#include <iostream>
#include <math.h>
#include <chrono>
#include <algorithm>

#include <SFML/Graphics.hpp>

#include "QuadTreeNode.h"
#include "Particle.h"
#include "ThreadPool.h"

void simulate(std::vector<Particle>& particles, QuadTreeNode& root, const double dt, ThreadPool& pool) {
    for (Particle& particle: particles) {
        pool.enqueue([&root, &particle] {
            Vector2d force = root.getTotalForce(&particle);
            particle.updateAcceletation(force);
        });
    }
    pool.waitForAll();
    for (Particle& particle : particles) {
        pool.enqueue([&particle, dt] {
            particle.move(dt);
        });
    }
    pool.waitForAll();
}

void updateVisualisation(std::vector<Particle>& particles, std::vector<sf::CircleShape>& particlesVisual, const sf::Vector2u windowDim, sf::RenderWindow& window, ThreadPool& pool) {
    for (int i = 0; i < particles.size(); i++) {
        pool.enqueue([&windowDim, &particle = particles[i], &particleVisual = particlesVisual[i]] {
            particleVisual.setPosition(sf::Vector2f(particle.position.x * windowDim.x, particle.position.y * windowDim.y));
        });    
    }
    pool.waitForAll();
    for (auto& particleVisual : particlesVisual) {
        window.draw(particleVisual);
    }
}

void insertParticles(std::vector<Particle>& particles, QuadTreeNode& root, ThreadPool& pool) {
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
    
    ThreadPool pool(std::thread::hardware_concurrency()/2);
    
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point begin_info = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point end;
    while (window.isOpen()) {
        end = std::chrono::steady_clock::now();
        double time_elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() / 1000000.f;
        std::cout << "fps: " << 1 / time_elapsed << '\n';
        begin = end;

        begin_info = end;
        insertParticles(particles, root, pool);
        end = std::chrono::steady_clock::now();
        std::cout << "build time: " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin_info).count() << '\n';

        begin_info = end;
        simulate(particles, root, time_elapsed * 0.01, pool);
        end = std::chrono::steady_clock::now();
        std::cout << "simulation time: " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin_info).count() << "\n \n";
        
        window.clear(sf::Color::Black);
        root.draw(window);
        updateVisualisation(particles, particlesVisual, windowDimentions, window, pool);
        window.display();

        //std::cin.get();
    }

    return 0;
}
