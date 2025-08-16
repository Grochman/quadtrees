#include "QuadTree.h"

QuadTree::QuadTree(std::vector<Particle>& particles)
    : _particles(particles) {
}

void QuadTree::simulate(const double dt) {
    for (Particle& particle : _particles) {
        Vector2d force = _root.getTotalForce(&particle);
        particle.updateAcceletation(force);
    }
    for (Particle& particle : _particles) {
        particle.move(dt);
    }
}

void QuadTree::build() {
    _root.reset();
    for (Particle& particle : _particles) {
        _root.addParticle(&particle);
    }
}

void QuadTree::computeBounds() {
    double left = INT_MAX;
    double right = INT_MIN;
    double top = INT_MAX;
    double bottom = INT_MIN;
    
    for (Particle& particle : _particles) {
        left = std::min(particle.position.x, left);
        right = std::max(particle.position.x, right);
        top = std::min(particle.position.y, top);
        bottom = std::max(particle.position.y, bottom);
    }

    _origin.x = left;
    _origin.y = top;
    _dimentions.x = right - left;
    _dimentions.y = bottom - top;
    
    _root.setBounds(_origin, _dimentions);
}

void QuadTree::draw(sf::RenderWindow& window, bool transform, bool showGrid, bool showMassCenter) {
    Vector2d scale;
    Vector2d translation;
    double particleScale;
    if (transform) {
        scale = { window.getSize().x / _dimentions.x, window.getSize().y / _dimentions.y };
        translation = { -_origin.x * scale.x , -_origin.y * scale.y };
        particleScale = 1 / _dimentions.x;
    }
    else {
        scale = { window.getSize().x / 1., window.getSize().y / 1. };
        translation = { 0. * scale.x , 0. * scale.y };
        particleScale = 1;
    }

    _root.draw(window, scale, translation, showGrid, showMassCenter);

    for (Particle& particle : _particles) {
        double radius = std::max(particle.mass * 5.f * particleScale, 1.);
        sf::CircleShape c(radius);
        c.setPosition(sf::Vector2f(particle.position.x * scale.x - radius + translation.x, particle.position.y * scale.y - radius + translation.y));
        c.setFillColor(sf::Color::White);
        window.draw(c);
    }
}
