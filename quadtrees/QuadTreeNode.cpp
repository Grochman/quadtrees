#include <math.h>
#include <ranges>
#include <iostream>

#include "QuadTreeNode.h"
#include "Particle.h"

QuadTreeNode::QuadTreeNode(const Vector2d& position, const Vector2d& dimentions): 
	_position(position), _dimentions(dimentions) {
}

QuadTreeNode::QuadTreeNode(const Vector2d& position, const Vector2d& dimentions, QuadTreeNode* const parent) :
	_position(position), _dimentions(dimentions), _parent(parent) {
}


QuadTreeNode::QuadTreeNode()
	: _position({ 0,0 }), _dimentions({ 1,1 }) {
}

QuadTreeNode::~QuadTreeNode() {
	deleteLeaves();
}

void QuadTreeNode::insertParticleToChildren(Particle* const particle) {
	if (particle->position.x > _position.x + _dimentions.w / 2) {
		if (particle->position.y > _position.y + _dimentions.h / 2) {
			_bottomRight->addParticle(particle);
		}
		else {
			_topRight->addParticle(particle);
		}
	}
	else {
		if (particle->position.y > _position.y + _dimentions.h / 2) {
			_bottomLeft->addParticle(particle);
		}
		else {
			_topLeft->addParticle(particle);
		}
	}
}

void QuadTreeNode::addParticle(Particle* const particle) {
	_particles.insert(particle);
	
	if (_topLeft != NULL) {
		insertParticleToChildren(particle);
		return;
	}
	
	particle->quad = this;
	
	if (_particles.size() == _maxCapacity) {
		split();
	}
}

void QuadTreeNode::split() {
	Vector2d new_dimentions = _dimentions / 2;
	_topLeft = new QuadTreeNode(_position, new_dimentions, this);
	_bottomLeft = new QuadTreeNode(Vector2d(_position.x, _position.y + _dimentions.h / 2), new_dimentions, this);
	_topRight = new QuadTreeNode(Vector2d(_position.x + _dimentions.w / 2, _position.y), new_dimentions, this);
	_bottomRight = new QuadTreeNode(Vector2d(_position.x + _dimentions.w / 2 , _position.y + _dimentions.h / 2), new_dimentions, this);
	for (Particle* particle: _particles) {
		insertParticleToChildren(particle);
	}
}

void QuadTreeNode::deleteLeaves() {
	delete _topLeft;
	delete _bottomLeft;
	delete _topRight;
	delete _bottomRight;
}

void QuadTreeNode::updateParticleOwnership(Particle* particle) {
	if (particle->position.x > _position.x && particle->position.y > _position.y
		&& particle->position.x < _position.x + _dimentions.w && particle->position.y < _position.y + _dimentions.h) {
		addParticle(particle);
		return;
	}

	_particles.erase(particle);
	if (_particles.size() < _maxCapacity) {
		if (_topLeft) {
			deleteLeaves();
		}
	}
	if (_parent) {
		_parent->updateParticleOwnership(particle);
	}
}

const std::unordered_set<Particle*>& QuadTreeNode::query(const Vector2d& position, const unsigned int radius) {
	Vector2d middle = _position + _dimentions / 2;
	const unsigned int distanceX = abs(middle.x - position.x);
	const unsigned int distanceY = abs(middle.y - position.y);

	const unsigned int distance = sqrt(pow(distanceX, 2) + pow(distanceY, 2));
	
	unsigned int scale;
	if (distanceX > distanceY) {
		scale = _dimentions.x / distanceX;
	}
	else {
		scale = _dimentions.y / distanceY;
	}
	const unsigned int squareRadius = distance * scale;
	
	if (radius + squareRadius >= distance) {
		if (_topLeft == NULL) {
			return _particles;
		}
		auto tl = _topLeft->query(position, radius);
		auto bl = _bottomLeft->query(position, radius);
		auto tr = _topRight->query(position, radius);
		auto br = _bottomRight->query(position, radius);

		std::unordered_set<Particle*>result;
		result.reserve(tl.size() + bl.size() + tr.size() + br.size());
		result.insert(tl.begin(), tl.end());
		result.insert(bl.begin(), bl.end());
		result.insert(tr.begin(), tr.end());
		result.insert(br.begin(), br.end());
		return result;
	}
	return std::unordered_set<Particle*>{};
}