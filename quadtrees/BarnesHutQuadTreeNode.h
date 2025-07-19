#pragma once
#include "QuadTreeNodeImp.h"
#include "Vector2d.h"
class BarnesHutQuadTreeNode : public QuadTreeNodeImp<1> {
	double _mass = 0;
	Vector2d _massCenter = {0,0};
	static constexpr float threshold = 0.5;
public:
	void addParticle(Particle* const particle) override;
	Vector2d query(Particle* const particle);
};

