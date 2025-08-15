#pragma once

#include <unordered_set>

#include "Vector2d.h"
#include <SFML/Graphics.hpp>
#include "Particle.h"

struct QuadTreeNode {
	Vector2d position = {0,0};
	double width = 1;
	int leavesIdx = -1;
	Particle* particle = nullptr;
    
	double mass = 0;
	Vector2d massCenter = { 0,0 };
};

bool contains(QuadTreeNode* node, Particle* particle) {
	if (particle->position.x >= node->position.x && particle->position.y >= node->position.y
		&& particle->position.x <= node->position.x + node->width && particle->position.y <= node->position.y + node->width) {
		return true;
	}
	return false;
}
