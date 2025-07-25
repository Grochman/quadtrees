#include <math.h>
#include <ranges>
#include <iostream>

#include "QuadTreeNode.h"
#include "Particle.h"


#define MIN 0.1

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
	if (!contains(*particle)) {
		return;
	}
	
	_particleCount++;
	
	if (_particle == particle) {
		return;
	}
	
	_massCenter = { (_massCenter.x * _mass) + (particle->position.x * particle->mass), (_massCenter.y * _mass) + (particle->position.y * particle->mass) };
	_mass += particle->mass;
	_massCenter.x /= _mass;
	_massCenter.y /= _mass;

	if (_topLeft != nullptr) {
		insertParticleToChildren(particle);
		return;
	}
	else if (!_particle) {
		particle->quad = this;
		_particle = particle;
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

void QuadTreeNode::draw(sf::RenderWindow& window) {	
	sf::RectangleShape r(sf::Vector2f(_dimentions.x * window.getSize().x, _dimentions.y * window.getSize().y));
	r.setPosition(sf::Vector2f( _position.x * window.getSize().x, _position.y * window.getSize().y));
	r.setOutlineThickness(1.f);
	r.setFillColor(sf::Color::Black);
	r.setOutlineColor(sf::Color(50,50,50));
	window.draw(r);
	
	if (_topLeft) {
		_topLeft->draw(window);
		_topRight->draw(window);
		_bottomLeft->draw(window);
		_bottomRight->draw(window);
	}
}

void QuadTreeNode::reset() {
	deleteLeaves();
	_mass = 0;
	_massCenter = { 0,0 };
}

Vector2d QuadTreeNode::getTotalForce(Particle* const particle) {
	Vector2d totalForce = { 0,0 };
	if (_particle == particle) {
		return totalForce;
	}
	if (_particle) {
		return calculateForce(_particle->position, _particle->mass, particle);
	}
	const double s = _dimentions.x;
	const double d = particle->position.distance(_massCenter).length();
	if (s/d < _thresshold) {
		return calculateForce(_massCenter, _mass, particle);
	}
	if (_topLeft) {
		totalForce += _topLeft->getTotalForce(particle);
		totalForce += _topRight->getTotalForce(particle);
		totalForce += _bottomLeft->getTotalForce(particle);
		totalForce += _bottomRight->getTotalForce(particle);
	}
	return totalForce;
}

Vector2d QuadTreeNode::calculateForce(const Vector2d& position, const double mass, Particle* const particle) {
	Vector2d distance = particle->position.distance(position);
	Vector2d unit_vector = distance.unit();
	double distance_length = distance.length();
	distance_length = std::max(distance_length, MIN);
	return { (mass * particle->mass / pow(distance_length, 2))* unit_vector.x, (mass * particle->mass / pow(distance_length, 2))* unit_vector.y };
}

bool QuadTreeNode::contains(Particle& const particle) {
	if (particle.position.x >= _position.x && particle.position.y >= _position.y
		&& particle.position.x <= _position.x + _dimentions.w && particle.position.y <= _position.y + _dimentions.h) {
		return true;
	}
	return false;
}