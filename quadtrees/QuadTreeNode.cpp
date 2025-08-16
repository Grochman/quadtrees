#include "QuadTreeNode.h"


bool contains(QuadTreeNode* node, Particle* particle) {
	if (particle->position.x >= node->position.x && particle->position.y >= node->position.y
		&& particle->position.x <= node->position.x + node->width && particle->position.y <= node->position.y + node->width) {
		return true;
	}
	return false;
}