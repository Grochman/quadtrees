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
	_depth = _parent->_depth + 1;
}


QuadTreeNode::QuadTreeNode()
	: _position({ 0,0 }), _dimentions({ 1,1 }) {
}

QuadTreeNode::~QuadTreeNode() {
	if (_topLeft) {
		deleteLeaves();
	}
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
	
	if (_topLeft != nullptr) {
		insertParticleToChildren(particle);
		return;
	}
	
	particle->quad = this;
	
	if (_particles.size() == _maxCapacity) {
		split();
	}
}

void QuadTreeNode::split() {
	if (_depth >= _maxDepth) {
		return;
	}
	Vector2d new_dimentions = _dimentions / 2;
	_topLeft = new QuadTreeNode(Vector2d(_position.x, _position.y), new_dimentions, this);
	_bottomLeft = new QuadTreeNode(Vector2d(_position.x, _position.y + _dimentions.h / 2), new_dimentions, this);
	_topRight = new QuadTreeNode(Vector2d(_position.x + _dimentions.w / 2, _position.y), new_dimentions, this);
	_bottomRight = new QuadTreeNode(Vector2d(_position.x + _dimentions.w / 2 , _position.y + _dimentions.h / 2), new_dimentions, this);
	for (Particle* particle: _particles) {
		insertParticleToChildren(particle);
	}
}

void QuadTreeNode::merge() {
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

void QuadTreeNode::deleteLeaves() {
	delete _topLeft;
	delete _bottomLeft;
	delete _topRight;
	delete _bottomRight;
	_topLeft = nullptr;
	_bottomLeft = nullptr;
	_topRight = nullptr;
	_bottomRight = nullptr;
}

void QuadTreeNode::updateParticleOwnership(Particle* particle) {
	if (_parent == nullptr) {
		addParticle(particle);
		return;
	}
	if (particle->position.x >= _position.x && particle->position.y >= _position.y
		&& particle->position.x <= _position.x + _dimentions.w && particle->position.y <= _position.y + _dimentions.h) {
		addParticle(particle);
		return;
	}

	_particles.erase(particle);
	particle->quad = _parent;
	if (_particles.size() < _maxCapacity) {
		merge();
	}
	_parent->updateParticleOwnership(particle);
	
}

std::unordered_set<Particle*> QuadTreeNode::query(const Vector2d& position, const double radius) {
	Vector2d middle = _position + _dimentions / 2;

	const double distanceX = abs(middle.x - position.x);
	const double distanceY = abs(middle.y - position.y);
	
	const double distance = sqrt(pow(distanceX, 2) + pow(distanceY, 2));
	
	unsigned int squareRadius = 0;
	unsigned int scale = 1;

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

void QuadTreeNode::draw(sf::RenderWindow& window) {	
	sf::RectangleShape r(sf::Vector2f(_dimentions.x * window.getSize().x, _dimentions.y * window.getSize().y));
	r.setPosition(sf::Vector2f( _position.x * window.getSize().x, _position.y * window.getSize().y));
	r.setOutlineThickness(1.f);
	r.setFillColor(sf::Color::Black);
	r.setOutlineColor(sf::Color(100,100,100));
	window.draw(r);
	
	if (_topLeft) {
		_topLeft->draw(window);
		_topRight->draw(window);
		_bottomLeft->draw(window);
		_bottomRight->draw(window);
	}
}