#include <iostream>

#include "QuadTree.h"
#include "Particle.h"


int main()
{
    srand(time(NULL));
    const unsigned int particleCount = 4;

    std::vector<Particle> particles;
    particles.reserve(particleCount);
    for (int i = 0; i < particleCount; i++) {
        particles.emplace_back();
        initParticle(&particles[i]);
    }
    
    QuadTree tree;
    
    while (true) {
        tree.reset();
        tree.insert(particles);
        tree.calculateMass();

        for (Particle& particle : particles) {
            tree.getTotalForce(particle);
        }
    }
    
    return 0;
}
