#pragma once

#include "Vector2d.h"
#include "Particle.h"


struct QuadTreeNode {
	float2 position = {0,0};
	float2 dimentions = { 1,1 };
	int leavesIdx = -1;
	Particle* particle = nullptr;
    
	float mass = 0;
	float2 massCenter = { 0,0 };
};

bool contains(QuadTreeNode* node, Particle* particle);
