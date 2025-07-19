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