#pragma once

#include <vector>

#include <SFML/Graphics.hpp>

#include "QuadTreeNode.h"
#include "Vector2d.h"
#include "Particle.h"


class QuadTree
{
    std::vector<QuadTreeNode> nodes;

    Vector2d calculateForce(Vector2d& distance, const double mass, Particle* particle);
public:
    QuadTree();
    void reset();
    void computeBounds();
    void insert(std::vector<Particle>& particles);
    void calculateMass();
    Vector2d getTotalForce(Particle& particle);
    void draw(sf::RenderWindow& window, const Vector2d& scale, const Vector2d& translation);
};
