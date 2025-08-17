#pragma once

#include <SFML/Graphics.hpp>

#include "Vector2d.h"
#include "Particle.h"

class QuadTreeNode {
    Vector2d _position;
    Vector2d _dimentions;
    QuadTreeNode* _parent = nullptr;
    QuadTreeNode* _leaves[4] = { nullptr };
    Particle* _particle = nullptr;
    double _mass = 0;
    static constexpr double _thresshold = 0.5;
    Vector2d _massCenter = { 0,0 };

    QuadTreeNode(const Vector2d& position, const Vector2d& dimentions, QuadTreeNode* const parent);
    Vector2d calculateForce(Vector2d& distance, const double mass, Particle* const particle);
    void deleteLeaves();
    void split();
    void insertParticleToChildren(Particle* const particle);
    bool contains(Particle& const particle);
    void drawBorder(sf::RenderWindow& window, const Vector2d& scale, const Vector2d& translation);
public:
    QuadTreeNode();
    ~QuadTreeNode();
    void addParticle(Particle* const particle);
    Vector2d getTotalForce(Particle* const particle);
    void setBounds(Vector2d& position, Vector2d& dimentions);
    void reset();
    void draw(sf::RenderWindow& window, const Vector2d& scale, const Vector2d& translation, bool showGrid, bool showMassCenter);
};
