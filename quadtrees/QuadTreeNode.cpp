#include "QuadTreeNode.h"


bool contains(QuadTreeNode* node, Particle* particle) {
	if (particle->position.x >= node->position.x && particle->position.y >= node->position.y
		&& particle->position.x <= node->position.x + node->dimentions.x && particle->position.y <= node->position.y + node->dimentions.y) {
		return true;
	}
	return false;
}