#pragma once

#include <vector>

#include "Particle.h"
#include "Vector2d.h"
#include <CL/cl.h>
#include <SFML/Graphics.hpp>

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
    std::vector<QuadTreeNode> _nodes;
    std::vector<Particle> &_particles;
    cl_context _context;
    cl_command_queue _queue;
    cl_mem _particleBuffer;
    cl_mem _nodeBuffer;
    size_t _globalDimensions;
    cl_kernel _updatePositionsKernel;
    cl_kernel _computeForcesKernel;
    void checkError(cl_int status, const char *what);
    void checkBuildError(cl_int status, cl_program program,
                         cl_device_id device);
    cl_kernel buildKernel(cl_device_id device, const char *source,
                          const char *functionName);
    bool contains(const QuadTreeNode *node, const Particle *particle);
    int childIndex(const QuadTreeNode *node, const Particle *particle);

  public:
    QuadTree(std::vector<Particle> &particles);
    void computeBounds();
    void build();
    void calculateMass();
    void computeForces();
    void updatePositions(float dt);
    void draw(sf::RenderWindow &window, bool transform = false,
              bool showGrid = false, bool showMassCenter = false);
};
