#pragma once

#include <unordered_set>

#include "Vector2d.h"
#include <SFML/Graphics.hpp>

struct Particle;

class QuadTreeNode {
protected:
    QuadTreeNode(const Vector2d& position, const Vector2d& dimentions, QuadTreeNode* const parent);
    void insertParticleToChildren(Particle* const particle);

    Vector2d _position;
    Vector2d _dimentions;
    QuadTreeNode* _parent = nullptr;
    QuadTreeNode* _topLeft = nullptr;
    QuadTreeNode* _topRight = nullptr;
    QuadTreeNode* _bottomLeft = nullptr;
    QuadTreeNode* _bottomRight = nullptr;
    std::unordered_set<Particle*> _particles;
    unsigned int _depth = 0;
    static constexpr unsigned int _maxDepth = INT_MAX;
public:
    QuadTreeNode(const Vector2d& position, const Vector2d& dimentions);
    QuadTreeNode();
    virtual ~QuadTreeNode() = default;
    virtual void addParticle(Particle* const particle);
    virtual void updateParticleOwnership(Particle* const particle) = 0;
    void draw(sf::RenderWindow& window);
};

