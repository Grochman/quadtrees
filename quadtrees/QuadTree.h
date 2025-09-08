#pragma once

#include <vector>

#include <SFML/Graphics.hpp>
#include <CL/cl.h>
#include "QuadTreeNode.h"
#include "Vector2d.h"
#include "Particle.h"


class QuadTree
{
    std::vector<QuadTreeNode> _nodes;
    std::vector<Particle>& _particles;
    cl_context _context;
    cl_command_queue _queue;
    cl_mem _particleBuffer;
    cl_mem _nodeBuffer;
    size_t _globalDimensions;
    cl_kernel _updatePositionsKernel;
    cl_kernel _computeForcesKernel;
    void checkError(cl_int status, cl_program program, cl_device_id device);
    cl_kernel buildKernel(cl_device_id device, const char* sourceFile, const char* functionName);
public:
    QuadTree(std::vector<Particle>& particles);
    void computeBounds();
    void build();
    void calculateMass();
    void computeForces();
    void updatePositions(float dt);
    void draw(sf::RenderWindow& window, bool transform = false, bool showGrid = false, bool showMassCenter = false);
};
