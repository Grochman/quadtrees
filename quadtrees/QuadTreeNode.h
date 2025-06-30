#pragma once

#include <unordered_set>

#include "Vector2d.h"

struct Particle;

class QuadTreeNode {
    QuadTreeNode(const Vector2d& position, const Vector2d& dimentions, QuadTreeNode* const parent);
    void deleteLeaves();
    void split();
    void insertParticleToChildren(Particle* const particle);

    Vector2d _position;
    Vector2d _dimentions;
    QuadTreeNode* _parent = NULL;
    QuadTreeNode* _topLeft = NULL;
    QuadTreeNode* _topRight = NULL;
    QuadTreeNode* _bottomLeft = NULL;
    QuadTreeNode* _bottomRight = NULL;
    unsigned int _maxCapacity = 4;
    std::unordered_set<Particle*> _particles;

public:
    QuadTreeNode(const Vector2d& position, const Vector2d& dimentions);
    QuadTreeNode();
    ~QuadTreeNode();
    void addParticle(Particle* const particle);
    void updateParticleOwnership(Particle* const particle);
    const std::unordered_set<Particle*>& query(const Vector2d& position, const unsigned int radius);

};

