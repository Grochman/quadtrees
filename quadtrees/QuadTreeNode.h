#pragma once

#include <unordered_set>

#include "Vector2d.h"
#include <SFML/Graphics.hpp>

struct Particle;

class QuadTreeNode {
    Vector2d _position;
    Vector2d _dimentions;
    QuadTreeNode* _parent = nullptr;
    QuadTreeNode* _topLeft = nullptr;
    QuadTreeNode* _topRight = nullptr;
    QuadTreeNode* _bottomLeft = nullptr;
    QuadTreeNode* _bottomRight = nullptr;
    Particle* _particle = nullptr;
    unsigned int _particleCount = 0;
    double _mass = 0;
    static constexpr double _thresshold = 0.5;
    Vector2d _massCenter = { 0,0 };

    QuadTreeNode(const Vector2d& position, const Vector2d& dimentions, QuadTreeNode* const parent);
    Vector2d calculateForce(const Vector2d& position, const double mass, Particle* const particle);
    void deleteLeaves();
    void split();
    void insertParticleToChildren(Particle* const particle);
    bool contains(Particle& const particle);
public:
    QuadTreeNode();
    ~QuadTreeNode();
    void addParticle(Particle* const particle);
    Vector2d getTotalForce(Particle* const particle);
    void reset();
    void draw(sf::RenderWindow& window);
};

