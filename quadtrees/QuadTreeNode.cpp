#include <math.h>
#include <ranges>
#include <iostream>

#include "QuadTreeNode.h"
#include "Particle.h"

QuadTreeNode::QuadTreeNode(const Vector2d& position, const Vector2d& dimentions): 
	_position(position), _dimentions(dimentions) {}

QuadTreeNode::QuadTreeNode(const Vector2d& position, const Vector2d& dimentions, QuadTreeNode* const parent) :
	_position(position), _dimentions(dimentions), _parent(parent) {}


QuadTreeNode::QuadTreeNode()
	: _position({ 0,0 }), _dimentions({ 1,1 }) {}

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
	particleCount++;
	if (_topLeft != nullptr) {
		insertParticleToChildren(particle);
		return;
	}
	
	if (!_particle) {
		particle->quad = this;
		_particle = particle;
		return;
	}
	if (_particle == particle) {
		return;
	}

	split();
	insertParticleToChildren(particle);
	insertParticleToChildren(_particle);
	_particle = nullptr;
}

void QuadTreeNode::split() {
	Vector2d new_dimentions = _dimentions / 2;
	_topLeft = new QuadTreeNode(Vector2d(_position.x, _position.y), new_dimentions, this);
	_bottomLeft = new QuadTreeNode(Vector2d(_position.x, _position.y + _dimentions.h / 2), new_dimentions, this);
	_topRight = new QuadTreeNode(Vector2d(_position.x + _dimentions.w / 2, _position.y), new_dimentions, this);
	_bottomRight = new QuadTreeNode(Vector2d(_position.x + _dimentions.w / 2 , _position.y + _dimentions.h / 2), new_dimentions, this);
}

void QuadTreeNode::merge() {
	if (_parent == nullptr) {
		return;
	}
	
	if (_topLeft && this) {
		if (_topLeft->_particle) {
			_particle = _topLeft->_particle;
		}
		else if(_bottomLeft->_particle) {
			_particle = _bottomLeft->_particle;
		}
		else if (_bottomRight->_particle) {
			_particle = _bottomRight->_particle;
		}
		else if (_topRight->_particle) {
			_particle = _topRight->_particle;
		}
		if (_particle) {
			_particle->quad = this;
		}
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
	particleCount--;
	if (particle->position.x >= _position.x && particle->position.y >= _position.y
		&& particle->position.x <= _position.x + _dimentions.w && particle->position.y <= _position.y + _dimentions.h) {
		addParticle(particle);
		return;
	}

	_particle = nullptr;
	if (particleCount == 1) {
		merge();
	}
	if (_parent) {
		particle->quad = _parent;
		_parent->updateParticleOwnership(particle);
	}
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