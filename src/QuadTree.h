#pragma once

#include <vector>

#include "Particle.h"
#include "Vector2d.h"

struct QuadTreeNode {
    float2 position = {0, 0};
    float2 dimentions = {1, 1};
    float2 massCenter = {0, 0};
    float mass = 0;
    int leavesIdx = -1;
    int particleIdx = -1;
    int _pad = 0;
};

static_assert(sizeof(QuadTreeNode) == 40,
              "QuadTreeNode layout must match the OpenCL kernel");

class QuadTree {
  public:
    QuadTree(std::vector<Particle> &particles);
    std::vector<QuadTreeNode> &getNodes() { return _nodes; }
    void computeBounds();
    void build();
    void calculateMass();

  private:
    std::vector<QuadTreeNode> _nodes;
    std::vector<Particle> &_particles;
    bool contains(const QuadTreeNode *node, const Particle *particle);
    int childIndex(const QuadTreeNode *node, const Particle *particle);
};
