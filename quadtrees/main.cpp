#include <iostream>
#include <chrono>
#include <ctime>
#include <cstdlib>
#include <SFML/Graphics.hpp>

#include "QuadTree.h"
#include "Particle.h"


int main() {
    srand(time(NULL));
    
#ifdef NDEBUG
    const unsigned int particleCount = 1024;
#else
    const unsigned int particleCount = 256;
#endif
    
    std::vector<Particle> particles;
    particles.reserve(particleCount);
    for (int i = 0; i < particleCount; i++) {
        particles.emplace_back();
        initParticle(&particles[i]);
    }

    QuadTree tree(particles);

    const sf::Vector2u windowDimentions = { 600,600 };
    sf::RenderWindow window(sf::VideoMode(windowDimentions), "QuadTrees");

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point beginFps = std::chrono::steady_clock::now();

    int frameCount = 0;

    while (window.isOpen()) {
        while (const auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
        }
        frameCount++;
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        float timeElapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() / 1000000.f;
        begin = end;
        
        tree.computeBounds(); // reduction
        tree.build(); // dick knows
        tree.calculateMass(); // reduction?
        // Apprioximately sort the bodies by spatial distnce missing
        tree.computeForces(); // simple paraler
        tree.updatePositions(timeElapsed * 0.01f); // simple parallel
        
        std::chrono::steady_clock::time_point endFps = std::chrono::steady_clock::now();
        float frameTime= std::chrono::duration_cast<std::chrono::microseconds>(endFps - beginFps).count() / 1000000.f;
        
        if (frameCount / frameTime > 60) {
            beginFps = endFps;
            std::cout << "fps: " << frameCount / frameTime << '\n';

            window.clear(sf::Color::Black);
            tree.draw(window, false, true, false);
            window.display();
            frameCount = 0;
        }
        
    }

    return 0;
}
