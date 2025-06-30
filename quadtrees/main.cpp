#include <iostream>

#include "QuadTreeNode.h"
#include "Particle.h"

int main()
{
    std::cout << "Hello World!\n";
    std::vector<Particle> particles = { {{0.4, 0.2}}, {{0.5, 0.6}}, {{0.3 ,0.7}}, {{0.6, 0.2}}, {{0.1, 0.3}} };
    QuadTreeNode root;
    for (Particle& particle : particles) {
        root.addParticle(&particle);
    }
    particles[0].move({ 1,1 });
    particles[1].move({ 1,1 });
}
