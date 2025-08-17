#pragma once

#include "Vector2d.h"
#include "Particle.h"


struct QuadTreeNode {
	Vector2d position = {0,0};
	Vector2d dimentions = { 1,1 };
	int leavesIdx = -1;
	Particle* particle = nullptr;
    
	double mass = 0;
	Vector2d massCenter = { 0,0 };
};

bool contains(QuadTreeNode* node, Particle* particle);
