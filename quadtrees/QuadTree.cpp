#include "QuadTree.h"

void QuadTree::simulate(const double dt) {
    for (Particle& particle : _particles) {
        Vector2d force = _root.getTotalForce(&particle);
        particle.updateAcceletation(force);
    }
    for (Particle& particle : _particles) {
        particle.move(dt);
    }
}

QuadTree::QuadTree(std::vector<Particle>& particles)
    : _particles(particles) {}

void QuadTree::build() {
    _root.reset();
    for (Particle& particle : _particles) {
        _root.addParticle(&particle);
    }
}

void QuadTree::draw(sf::RenderWindow& window) {
    _root.draw(window);
}