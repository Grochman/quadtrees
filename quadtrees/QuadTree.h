#pragma once

#include <SFML/Graphics.hpp>

#include "QuadTreeNode.h"
#include "Particle.h"

class QuadTree
{
	QuadTreeNode _root;
	std::vector<Particle> _particles;
	Vector2d _dimentions;
	Vector2d _origin;
public:
	QuadTree(std::vector<Particle>& particles);
	void computeBounds();
	void build();
	void simulate(const double dt);
	void draw(sf::RenderWindow& window, bool transform = false, bool showGrid = false, bool showMassCenter = false);
};
