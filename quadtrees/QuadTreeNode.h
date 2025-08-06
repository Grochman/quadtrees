#pragma once

#include <unordered_set>

#include "Vector2d.h"
#include <SFML/Graphics.hpp>
#include "Particle.h"

struct QuadTreeNode {
	Vector2d position = {0,0};
	Vector2d dimentions = {1,1};
    QuadTreeNode* parent = nullptr;
    QuadTreeNode* topLeft = nullptr;
    QuadTreeNode* topRight = nullptr;
    QuadTreeNode* bottomLeft = nullptr;
    QuadTreeNode* bottomRight = nullptr;
    Particle* particle = nullptr;
    unsigned int particleCount = 0;
    double mass = 0;
    static constexpr double thresshold = 0.5;
    Vector2d massCenter = { 0,0 };
};

void initQuadNode(QuadTreeNode* node, const Vector2d& position, const Vector2d& dimentions, QuadTreeNode* const parent);
void freeQuad(QuadTreeNode* node);
void deleteLeaves(QuadTreeNode* node);
void insertParticleToChildren(QuadTreeNode* node, Particle* const particle);
void addParticle(QuadTreeNode* node, Particle* const particle);
void split(QuadTreeNode* node);
bool contains(QuadTreeNode* node, Particle& const particle);
Vector2d getTotalForce(QuadTreeNode* node, Particle* const particle);
Vector2d calculateForce(const Vector2d& position, const double mass, Particle* const particle);

#define MIN 0.1

void initQuadNode(QuadTreeNode* node, const Vector2d& position, const Vector2d& dimentions, QuadTreeNode* const parent) {
	node->position = position;
	node->dimentions = dimentions;
	node->parent = parent;
}

void freeQuad(QuadTreeNode* node) {
	if (node->topLeft) {
		freeQuad(node->bottomLeft);
		freeQuad(node->bottomRight);
		freeQuad(node->topLeft);
		freeQuad(node->topRight);
		deleteLeaves(node);
	}
}

void deleteLeaves(QuadTreeNode* node) {
	delete node->topLeft;
	delete node->bottomLeft;
	delete node->topRight;
	delete node->bottomRight;
	node->topLeft = nullptr;
	node->bottomLeft = nullptr;
	node->topRight = nullptr;
	node->bottomRight = nullptr;
}

void insertParticleToChildren(QuadTreeNode* node, Particle* const particle) {
	QuadTreeNode* child = node->topLeft;
	if (particle->position.x > node->position.x + node->dimentions.w / 2) {
		if (particle->position.y > node->position.y + node->dimentions.h / 2) {
			child = node->bottomRight;
		}
		else {
			child = node->topRight;
		}
	}
	else if (particle->position.y > node->position.y + node->dimentions.h / 2) {
		child = node->bottomLeft;
	}
	addParticle(child, particle);

}

void addParticle(QuadTreeNode* node, Particle* const particle) {
	if (!contains(node, *particle)) {
		return;
	}

	node->particleCount++;

	if (node->particle == particle) {
		return;
	}

	node->massCenter = { (node->massCenter.x * node->mass) + (particle->position.x * particle->mass), (node->massCenter.y * node->mass) + (particle->position.y * particle->mass) };
	node->mass += particle->mass;
	node->massCenter.x /= node->mass;
	node->massCenter.y /= node->mass;

	if (node->topLeft != nullptr) {
		insertParticleToChildren(node, particle);
		return;
	}
	else if (!node->particle) {
		particle->quad = node;
		node->particle = particle;
		return;
	}

	split(node);
	insertParticleToChildren(node, particle);
	insertParticleToChildren(node, node->particle);
	node->particle = nullptr;
}

void split(QuadTreeNode* node) {
	Vector2d new_dimentions = divide(node->dimentions, 2);
	node->topLeft = new	QuadTreeNode;
	node->bottomLeft = new QuadTreeNode;
	node->topRight = new QuadTreeNode;
	node->bottomRight = new QuadTreeNode;
	initQuadNode(node->topLeft, Vector2d{ node->position.x, node->position.y }, new_dimentions, node);
	initQuadNode(node->bottomLeft, Vector2d{ node->position.x, node->position.y + node->dimentions.h / 2 }, new_dimentions, node);
	initQuadNode(node->topRight, Vector2d{ node->position.x + node->dimentions.w / 2, node->position.y }, new_dimentions, node);
	initQuadNode(node->bottomRight, Vector2d{ node->position.x + node->dimentions.w / 2, node->position.y + node->dimentions.h / 2 }, new_dimentions, node);
}

void draw(QuadTreeNode* node, sf::RenderWindow& window) {
	sf::RectangleShape r(sf::Vector2f(node->dimentions.x * window.getSize().x, node->dimentions.y * window.getSize().y));
	r.setPosition(sf::Vector2f(node->position.x * window.getSize().x, node->position.y * window.getSize().y));
	r.setOutlineThickness(1.f);
	r.setFillColor(sf::Color::Black);
	r.setOutlineColor(sf::Color(50, 50, 50));
	window.draw(r);

	if (node->topLeft) {
		draw(node->topLeft, window);
		draw(node->topRight, window);
		draw(node->bottomRight, window);
		draw(node->bottomLeft, window);
	}
}

void reset(QuadTreeNode* node) {
	freeQuad(node);
	node->mass = 0;
	node->massCenter = { 0,0 };
}

Vector2d getTotalForce(QuadTreeNode* node, Particle* const particle) {
	Vector2d totalForce = { 0,0 };
	if (node->particle == particle) {
		return totalForce;
	}
	if (node->particle) {
		return calculateForce(node->particle->position, node->particle->mass, particle);
	}
	const double s = node->dimentions.x;
	const double d = length(distance(particle->position, node->massCenter));
	if (s / d < node->thresshold) {
		return calculateForce(node->massCenter, node->mass, particle);
	}
	if (node->topLeft) {
		totalForce = sum(totalForce, getTotalForce(node->topLeft, particle));
		totalForce = sum(totalForce, getTotalForce(node->topRight, particle));
		totalForce = sum(totalForce, getTotalForce(node->bottomLeft, particle));
		totalForce = sum(totalForce, getTotalForce(node->bottomRight, particle));
	}
	return totalForce;
}

Vector2d calculateForce(const Vector2d& position, const double mass, Particle* const particle) {
	Vector2d dist = distance(particle->position, position);
	Vector2d unit_vector = unit(dist);
	double distance_length = length(dist);
	distance_length = std::max(distance_length, MIN);
	return { (mass * particle->mass / pow(distance_length, 2)) * unit_vector.x, (mass * particle->mass / pow(distance_length, 2)) * unit_vector.y };
}

bool contains(QuadTreeNode* node, Particle& const particle) {
	if (particle.position.x >= node->position.x && particle.position.y >= node->position.y
		&& particle.position.x <= node->position.x + node->dimentions.w && particle.position.y <= node->position.y + node->dimentions.h) {
		return true;
	}
	return false;
}
