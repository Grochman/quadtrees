#include "QuadTree.h"

#include <iostream>
#include <stack>

#define MIN 0.1

QuadTree::QuadTree(const std::vector<Particle>& particles) 
    : _particles(particles){};

Vector2d QuadTree::calculateForce(Vector2d& distance, const double mass, Particle* particle) {
    Vector2d unit_vector = unit(distance);
    double distance_length = length(distance);
    distance_length = std::max(distance_length, MIN);
    return { (mass * particle->mass / pow(distance_length, 2)) * unit_vector.x, (mass * particle->mass / pow(distance_length, 2)) * unit_vector.y };
}

void QuadTree::computeBounds() {
    _nodes.clear();
    _nodes.push_back(QuadTreeNode());

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

    _nodes[0].position.x = left;
    _nodes[0].position.y = top;
    _nodes[0].width = std::max(right - left, bottom - top);
}

void QuadTree::build() {
    _nodes.clear();
    _nodes.push_back(QuadTreeNode());

    // while particles insert
    for (Particle& particle : _particles) {
        if (!contains(&_nodes[0], &particle)) {
            continue;
        }

        // go down the tree
        int idx = 0;
        while (_nodes[idx].leavesIdx != -1) {
            int newIdx = -1;
            for (int i = 0; i < 4; i++) {
                unsigned int leafIdx = _nodes[idx].leavesIdx + i;
                if (contains(&_nodes[leafIdx], &particle)) {
                    newIdx = leafIdx;
                }
            }
            idx = newIdx;
        }

        // split if needed
        while (_nodes[idx].particle) {
            _nodes[idx].leavesIdx = _nodes.size();
            Particle* p = _nodes[idx].particle;
            _nodes[idx].particle = nullptr;

            for (int i = 0; i < 4; i++) {
                _nodes.push_back(QuadTreeNode());
                int newIdx = _nodes.size() - 1;
                _nodes[newIdx].width = _nodes[idx].width / 2;
                switch (i) {
                case 0:
                    _nodes[newIdx].position = _nodes[idx].position;
                    break;
                case 1:
                    _nodes[newIdx].position = { _nodes[idx].position.x + _nodes[newIdx].width, _nodes[idx].position.y };
                    break;
                case 2:
                    _nodes[newIdx].position = { _nodes[idx].position.x, _nodes[idx].position.y + _nodes[newIdx].width};
                    break;
                default:
                    _nodes[newIdx].position = { _nodes[idx].position.x + _nodes[newIdx].width, _nodes[idx].position.y + _nodes[newIdx].width };
                }
                if (contains(&_nodes[newIdx], p)) {
                    _nodes[newIdx].particle = p;
                }
            }
            // go to correct quad
            for (int i = 0; i < 4; i++) {
                int leafIdx = _nodes[idx].leavesIdx + i;
                if (contains(&_nodes[leafIdx], &particle)) {
                    idx = leafIdx;
                    break;
                }
            }
        }
        // insert
        _nodes[idx].particle = &particle;
    }
}

void QuadTree::calculateMass() {
    for (int i = _nodes.size() - 1; i >= 0; i--) {
        if (_nodes[i].particle) {
            _nodes[i].mass = _nodes[i].particle->mass;
            _nodes[i].massCenter = _nodes[i].particle->position;
        }
        else if (_nodes[i].leavesIdx == -1) {
            _nodes[i].mass = 0;
            _nodes[i].massCenter = { 0,0 };
        }
        else {
            _nodes[i].massCenter = _nodes[_nodes[i].leavesIdx].massCenter;
            _nodes[i].mass = _nodes[_nodes[i].leavesIdx].mass;
            for (int j = 1; j <= 3; j++) {
                _nodes[i].massCenter = { _nodes[i].massCenter.x * _nodes[i].mass +
                    _nodes[_nodes[i].leavesIdx + j].massCenter.x * _nodes[_nodes[i].leavesIdx + j].mass,
                    _nodes[i].massCenter.y * _nodes[i].mass +
                    _nodes[_nodes[i].leavesIdx + j].massCenter.y * _nodes[_nodes[i].leavesIdx + j].mass };
                _nodes[i].mass += _nodes[_nodes[i].leavesIdx + j].mass;
                if (_nodes[i].mass != 0) {
                    _nodes[i].massCenter.x /= _nodes[i].mass;
                    _nodes[i].massCenter.y /= _nodes[i].mass;
                }
            }
        }
    }
}

Vector2d QuadTree::getTotalForce(Particle& particle) {
    Vector2d totalForce = { 0,0 };

    std::stack<int> idxs;
    idxs.push(0);

    while (!idxs.empty()) {
        int i = idxs.top();
        idxs.pop();

        if (_nodes[i].particle == &particle) {
            continue;
        }
        if (_nodes[i].particle) {
            Vector2d r = distance(particle.position, _nodes[i].particle->position);
            totalForce = sum(totalForce, calculateForce(r, _nodes[i].particle->mass, &particle));
            continue;
        }
        const double s = _nodes[i].width;
        const double d = length(distance(particle.position, _nodes[i].massCenter));
        if (s / d < 0.5) {
            Vector2d r = distance(particle.position, _nodes[i].massCenter);
            totalForce = sum(totalForce, calculateForce(r, _nodes[i].mass, &particle));
            continue;
        }
        else if (_nodes[i].leavesIdx != -1) {
            idxs.push(_nodes[i].leavesIdx);
            idxs.push(_nodes[i].leavesIdx + 1);
            idxs.push(_nodes[i].leavesIdx + 2);
            idxs.push(_nodes[i].leavesIdx + 3);
        }
    }

    return totalForce;
}

void QuadTree::computeForces() {
    for (Particle& particle : _particles) {
        Vector2d force = getTotalForce(particle);
        updateAcceletation(&particle, force);
    }
}

void QuadTree::updatePositions(double dt) {
    for (Particle& particle : _particles) {
        move(&particle, dt);
    }
}

void QuadTree::draw(sf::RenderWindow& window, bool transform, bool showGrid, bool showMassCenter) {
    Vector2d scale;
    Vector2d translation;
    double particleScale;
    if (transform) {
        scale = { window.getSize().x / _nodes[0].width, window.getSize().y / _nodes[0].width };
        translation = { -_nodes[0].position.x * scale.x , -_nodes[0].position.y * scale.y};
        particleScale = 1 / _nodes[0].width;
    }
    else {
        scale = { window.getSize().x / 1., window.getSize().y / 1. };
        translation = { 0. * scale.x , 0. * scale.y };
        particleScale = 1;
    }
    
    if (showGrid) {
        for (QuadTreeNode& node : _nodes) {
            sf::RectangleShape r(sf::Vector2f(node.width * scale.x, node.width * scale.y));
            r.setPosition(sf::Vector2f(node.position.x * scale.x + translation.x, node.position.y * scale.y + translation.y));
            r.setOutlineThickness(1.f);
            r.setFillColor(sf::Color::Black);
            r.setOutlineColor(sf::Color(50, 50, 50));
            window.draw(r);
        }
    }
    
    for (Particle& particle : _particles) {
        double radius = std::max(particle.mass * 5.f * particleScale, 1.);
        sf::CircleShape c(radius);
        c.setPosition(sf::Vector2f(particle.position.x * scale.x - radius + translation.x, particle.position.y * scale.y - radius + translation.y));
        c.setFillColor(sf::Color::White);
        window.draw(c);
    }

    if (showMassCenter) {
        int radius = 5;
        sf::CircleShape c(radius);
        c.setPosition(sf::Vector2f(_nodes[0].massCenter.x * scale.x - radius + translation.x, _nodes[0].massCenter.y * scale.y - radius + translation.y));
        c.setFillColor(sf::Color::Red);
        window.draw(c);
    }
}