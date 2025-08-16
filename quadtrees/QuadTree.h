#pragma once

#include <vector>

#include <SFML/Graphics.hpp>

#include "QuadTreeNode.h"
#include "Vector2d.h"
#include "Particle.h"


class QuadTree
{
    std::vector<QuadTreeNode> _nodes;
    std::vector<Particle> _particles;

    Vector2d calculateForce(Vector2d& distance, const double mass, Particle* particle);
    
public:
    QuadTree(const std::vector<Particle>& particles);
    void computeBounds();
    void build();
    void calculateMass();
    void computeForces();
    void updatePositions(double dt);
    Vector2d getTotalForce(Particle& particle);
    void draw(sf::RenderWindow& window, bool transform = false, bool showGrid = false, bool showMassCenter = false);
};
