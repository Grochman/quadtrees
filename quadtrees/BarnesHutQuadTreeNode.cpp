#include "BarnesHutQuadTreeNode.h"
void BarnesHutQuadTreeNode::addParticle(Particle* const particle) {
	_massCenter.x = (_massCenter.x * _mass) + (particle->position.x * particle->mass);
	_massCenter.y = (_massCenter.y * _mass) + (particle->position.y * particle->mass);

	_mass += particle->mass;

	_massCenter.x /= _mass;
	_massCenter.y /= _mass;

	QuadTreeNode::addParticle(particle);
}

// removing particle

Vector2d BarnesHutQuadTreeNode::query(Particle* const particle) {
	if (_particles.size() == 1 && *_particles.begin() != particle) {
		return *_particles.begin();
	}
	
	double s = _dimentions.x;
	double d = _massCenter.distance(particle->position).length();
	if (d / s < threshold) {
		return Particle(_massCenter, _mass); // calculate force not particle
	}
	return Particle(_massCenter, _mass);
}

