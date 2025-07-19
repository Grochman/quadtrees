#pragma once

#include <unordered_set>

#include "Vector2d.h"
#include <SFML/Graphics.hpp>

struct Particle;

class QuadTreeNode {
    QuadTreeNode(const Vector2d& position, const Vector2d& dimentions, QuadTreeNode* const parent);
    void deleteLeaves();
    void split();
    void merge();
    void insertParticleToChildren(Particle* const particle);

    Vector2d _position;
    Vector2d _dimentions;
    QuadTreeNode* _parent = nullptr;
    QuadTreeNode* _topLeft = nullptr;
    QuadTreeNode* _topRight = nullptr;
    QuadTreeNode* _bottomLeft = nullptr;
    QuadTreeNode* _bottomRight = nullptr;
    Particle* _particle = nullptr;
    unsigned int particleCount = 0;
public:
    QuadTreeNode(const Vector2d& position, const Vector2d& dimentions);
    QuadTreeNode();
    ~QuadTreeNode();
    void addParticle(Particle* const particle);
    void updateParticleOwnership(Particle* const particle);
    void draw(sf::RenderWindow& window);
};

