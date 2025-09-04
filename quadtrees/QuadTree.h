#pragma once

#include <vector>

#include <SFML/Graphics.hpp>

#include "QuadTreeNode.h"
#include "Vector2d.h"
#include "Particle.h"


class QuadTree
{
    std::vector<QuadTreeNode> _nodes;
    std::vector<Particle>& _particles;

    float2 calculateForce(float2& distance, const float mass, Particle* particle);
    
public:
    QuadTree(std::vector<Particle>& particles);
    void computeBounds();
    void build();
    void calculateMass();
    void computeForces();
    void updatePositions(double dt);
    float2 getTotalForce(Particle& particle);
    void draw(sf::RenderWindow& window, bool transform = false, bool showGrid = false, bool showMassCenter = false);
};
