#include <iostream>
#include <chrono>
#include <random>

#include <SFML/Graphics.hpp>

#include "QuadTree.h"
#include "Particle.h"

int main()
{
    srand(time(NULL));
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<> dis(0.5, 0.2);
    
    const unsigned int particleCount = 500;

    std::vector<Particle> particles;
    particles.reserve(particleCount);
    for (int i = 0; i < particleCount; i++) {
        particles.emplace_back(Vector2d{dis(gen), dis(gen)}, Vector2d{ double(rand()) / RAND_MAX - 0.5, double(rand()) / RAND_MAX - 0.5 }, dis(gen));
    }

    QuadTree tree(particles);
    
    const sf::Vector2u windowDimentions = { 600,600 };
    sf::RenderWindow window(sf::VideoMode(windowDimentions), "QuadTrees");
    
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    while (window.isOpen()) {
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        double timeElapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() / 1000000.f;
        //std::cout << "fps: " << 1 / timeElapsed << '\n';
        begin = end;

        tree.computeBounds();
        tree.build();
        tree.simulate(timeElapsed * 0.01);
        
        window.clear(sf::Color::Black);
        tree.draw(window, false, true);
        window.display();
    }

    return 0;
}
