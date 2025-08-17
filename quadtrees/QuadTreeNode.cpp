#include "QuadTreeNode.h"

#define MIN 0.1

QuadTreeNode::QuadTreeNode(const Vector2d& position, const Vector2d& dimentions, QuadTreeNode* const parent) :
	_position(position), _dimentions(dimentions), _parent(parent) {}

QuadTreeNode::QuadTreeNode()
	: _position({ 0,0 }), _dimentions({ 1,1 }) {}

QuadTreeNode::~QuadTreeNode() {
	deleteLeaves();
}

void QuadTreeNode::setBounds(Vector2d& position, Vector2d& dimentions) {
	_position = position;
	_dimentions = dimentions;
}

void QuadTreeNode::insertParticleToChildren(Particle* const particle) {
	if (particle->position.x > _position.x + _dimentions.w / 2) {
		if (particle->position.y > _position.y + _dimentions.h / 2) {
			_leaves[3]->addParticle(particle);
		}
		else {
			_leaves[1]->addParticle(particle);
		}
	}
	else {
		if (particle->position.y > _position.y + _dimentions.h / 2) {
			_leaves[2]->addParticle(particle);
		}
		else {
			_leaves[0]->addParticle(particle);
		}
	}
}

void QuadTreeNode::addParticle(Particle* const particle) {
	if (_particle == particle) {
		return;
	}
	
	_massCenter = { (_massCenter.x * _mass) + (particle->position.x * particle->mass), (_massCenter.y * _mass) + (particle->position.y * particle->mass) };
	_mass += particle->mass;
	_massCenter.x /= _mass;
	_massCenter.y /= _mass;

	if (!contains(*particle)) {
		return;
	}

	if (_leaves[0]) {
		insertParticleToChildren(particle);
		return;
	}
	else if (!_particle) {
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
	_leaves[0] = new QuadTreeNode(Vector2d(_position.x, _position.y), new_dimentions, this);
	_leaves[2] = new QuadTreeNode(Vector2d(_position.x, _position.y + _dimentions.h / 2), new_dimentions, this);
	_leaves[1] = new QuadTreeNode(Vector2d(_position.x + _dimentions.w / 2, _position.y), new_dimentions, this);
	_leaves[3] = new QuadTreeNode(Vector2d(_position.x + _dimentions.w / 2 , _position.y + _dimentions.h / 2), new_dimentions, this);
}

void QuadTreeNode::deleteLeaves() {
	for (QuadTreeNode*& leaf : _leaves) {
		delete leaf;
		leaf = nullptr;
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
		Vector2d r = particle->position.distance(_particle->position);
		return calculateForce(r, _particle->mass, particle);
	}
	const double s = _dimentions.x;
	const double d = particle->position.distance(_massCenter).length();
	if (s/d < _thresshold) {
		Vector2d r = particle->position.distance(_massCenter);
		return calculateForce(r, _mass, particle);
	}
	else if (_leaves[0]) {
		for (QuadTreeNode*& leaf : _leaves) {
			totalForce += leaf->getTotalForce(particle);
		}
	}
	return totalForce;
}

Vector2d QuadTreeNode::calculateForce(Vector2d& distance, const double mass, Particle* const particle) {
	Vector2d unit_vector = distance.unit();
	double distance_length = distance.length();
	distance_length = std::max(distance_length, MIN);
	return { (mass * particle->mass / pow(distance_length, 2)) * unit_vector.x, (mass * particle->mass / pow(distance_length, 2)) * unit_vector.y };
}

bool QuadTreeNode::contains(Particle& const particle) {
	if (particle.position.x >= _position.x && particle.position.y >= _position.y
		&& particle.position.x <= _position.x + _dimentions.w && particle.position.y <= _position.y + _dimentions.h) {
		return true;
	}
	return false;
}

void QuadTreeNode::draw(sf::RenderWindow& window, const Vector2d& scale, const Vector2d& translation, bool showGrid, bool showMassCenter ) {
	if (_parent) {
		return;
	}

	if (showGrid) {
		drawBorder(window, scale, translation);
	}

	if (showMassCenter) {
		int radius = 5;
		sf::CircleShape c(radius);
		c.setPosition(sf::Vector2f(_massCenter.x * scale.x - radius + translation.x, _massCenter.y * scale.y - radius + translation.y));
		c.setFillColor(sf::Color::Red);
		window.draw(c);
	}
}

void QuadTreeNode::drawBorder(sf::RenderWindow& window, const Vector2d& scale, const Vector2d& translation) {
	sf::RectangleShape r(sf::Vector2f(_dimentions.x  * scale.x, _dimentions.y  * scale.y));
	r.setPosition(sf::Vector2f(_position.x * scale.x + translation.x, _position.y * scale.y + translation.y));
	r.setOutlineThickness(1.f);
	r.setFillColor(sf::Color::Black);
	r.setOutlineColor(sf::Color(50, 50, 50));
	window.draw(r);

	if (_leaves[0]) {
		for (QuadTreeNode*& leaf : _leaves) {
			leaf->drawBorder(window, scale, translation);
		}
	}
}
