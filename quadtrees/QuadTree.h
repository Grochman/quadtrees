#pragma once
#include "QuadTreeNode.h"
#include "Particle.h"

class QuadTree
{
	QuadTreeNode _root;
	std::vector<Particle> _particles;
public:
	QuadTree(std::vector<Particle>& particles);
	void build();
	void simulate(const double dt);
	void draw(sf::RenderWindow& window);

};

