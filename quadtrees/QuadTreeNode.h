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
    unsigned int _maxCapacity = 1;
    std::unordered_set<Particle*> _particles;
    unsigned int _depth = 0;
    static const unsigned int _maxDepth = 100;
public:
    QuadTreeNode(const Vector2d& position, const Vector2d& dimentions);
    QuadTreeNode();
    ~QuadTreeNode();
    void addParticle(Particle* const particle);
    void updateParticleOwnership(Particle* const particle);
    std::unordered_set<Particle*> query(const Vector2d& position, const double radius);
    void draw(sf::RenderWindow& window);
};

