#pragma once
#include "Particle.h"
#include "QuadTreeNodeImp.h"

template<unsigned int MaxCapacity>
void QuadTreeNodeImp<MaxCapacity>::addParticle(Particle* const particle) {
	QuadTreeNode::addParticle(particle);

	if (_particles.size() > _maxCapacity) {
		split();
	}
}


template<unsigned int MaxCapacity>
void QuadTreeNodeImp<MaxCapacity>::updateParticleOwnership(Particle* const particle) {
	if (particle->position.x >= _position.x && particle->position.y >= _position.y
		&& particle->position.x <= _position.x + _dimentions.w && particle->position.y <= _position.y + _dimentions.h) {
		addParticle(particle);
		return;
	}
	
	_particles.erase(particle);
	particle->quad = _parent;
	if (_particles.size() <= _maxCapacity) {
		merge();
	}

	if (_parent) {
		_parent->updateParticleOwnership(particle);
	}
}


template<unsigned int MaxCapacity>
void QuadTreeNodeImp<MaxCapacity>::split() {
	if (_depth > _maxDepth) {
		return;
	}
	if (_topLeft) {
		deleteLeaves();
	}
	Vector2d new_dimentions = _dimentions / 2;
	_topLeft = new QuadTreeNodeImp<MaxCapacity>(Vector2d(_position.x, _position.y), new_dimentions, this);
	_bottomLeft = new QuadTreeNodeImp<MaxCapacity>(Vector2d(_position.x, _position.y + _dimentions.h / 2), new_dimentions, this);
	_topRight = new QuadTreeNodeImp<MaxCapacity>(Vector2d(_position.x + _dimentions.w / 2, _position.y), new_dimentions, this);
	_bottomRight = new QuadTreeNodeImp<MaxCapacity>(Vector2d(_position.x + _dimentions.w / 2, _position.y + _dimentions.h / 2), new_dimentions, this);
	for (Particle* particle : _particles) {
		insertParticleToChildren(particle);
	}
}

template<unsigned int MaxCapacity>
void QuadTreeNodeImp<MaxCapacity>::deleteLeaves() {
	delete _topLeft;
	delete _bottomLeft;
	delete _topRight;
	delete _bottomRight;
	_topLeft = nullptr;
	_bottomLeft = nullptr;
	_topRight = nullptr;
	_bottomRight = nullptr;
}

template<unsigned int MaxCapacity>
void QuadTreeNodeImp<MaxCapacity>::merge() {
	if (_parent == nullptr) {
		return;
	}
	for (Particle* particle : _particles) {
		particle->quad = this;
	}
	if (_topLeft && this) {
		deleteLeaves();
	}
}

template<unsigned int MaxCapacity>
QuadTreeNodeImp<MaxCapacity>::~QuadTreeNodeImp() {
	_particles.clear();
	if (_topLeft) {
		deleteLeaves();
	}
}

template<unsigned int MaxCapacity>
std::unordered_set<Particle*> QuadTreeNodeImp<MaxCapacity>::query(const Vector2d& position, const double radius) {
	Vector2d middle = _position + _dimentions / 2;

	const double distanceX = abs(middle.x - position.x);
	const double distanceY = abs(middle.y - position.y);

	const double distance = sqrt(pow(distanceX, 2) + pow(distanceY, 2));

	double squareRadius = 0;
	double scale = 1;

	if (distanceX != 0 || distanceY != 0) {
		if (distanceX >= distanceY) {
			scale = _dimentions.x / distanceX;
		}
		else if (distanceX < distanceY) {
			scale = _dimentions.y / distanceY;
		}
		squareRadius = distance * scale;
	}

	if (radius + squareRadius >= distance) {
		if (_topLeft == nullptr) {
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
	return {};
}
