#pragma once

#include "Vector2d.h"
#include "Particle.h"


struct QuadTreeNode {
	float2 position = {0,0};
	float2 dimentions = { 1,1 };
	float2 massCenter = { 0,0 };
	float mass = 0;
	int leavesIdx = -1;
	int particleIdx = -1;
};

bool contains(QuadTreeNode* node, Particle* particle);
