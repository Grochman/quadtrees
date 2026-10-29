#include "QuadTree.h"
#include <iostream>
#include <fstream>
#include <limits>
#include <cmath>
#include <algorithm>
#include <omp.h>

#define MIN 0.1f
#define ROUNDING_ERROR_CORRECTION 0.0001f
#define THETA 0.5f
#define MAX_STACK 64

QuadTree::QuadTree(std::vector<Particle>& particles, ComputeBackend backend)
    : _particles(particles), _backend(backend) {
#ifdef QUADTREES_ENABLE_OPENCL
    if (_backend == ComputeBackend::OpenCL) {
        initOpenCL();
    }
#endif
}

QuadTree::~QuadTree() {
#ifdef QUADTREES_ENABLE_OPENCL
    if (_computeForcesKernel) {
        clReleaseKernel(_computeForcesKernel);
    }
    if (_updatePositionsKernel) {
        clReleaseKernel(_updatePositionsKernel);
    }
    if (_particleBuffer) {
        clReleaseMemObject(_particleBuffer);
    }
    if (_nodeBuffer) {
        clReleaseMemObject(_nodeBuffer);
    }
    if (_queue) {
        clReleaseCommandQueue(_queue);
    }
    if (_context) {
        clReleaseContext(_context);
    }
#endif
}

#ifdef QUADTREES_ENABLE_OPENCL
void QuadTree::initOpenCL() {
    cl_platform_id platfmors;
    clGetPlatformIDs(1, &platfmors, nullptr);
    cl_device_id device;
    clGetDeviceIDs(platfmors, CL_DEVICE_TYPE_GPU, 1, &device, nullptr);

    _context = clCreateContext(nullptr, 1, &device, nullptr, nullptr, nullptr);
    _queue = clCreateCommandQueueWithProperties(_context, device, nullptr, nullptr);

    _particleBuffer = clCreateBuffer(_context, CL_MEM_READ_WRITE, _particles.size() * sizeof(Particle), nullptr, nullptr);
    _globalDimensions = _particles.size();

    _updatePositionsKernel = buildKernel(device, "kernels/updatePositions.cl", "updatePositions");
    _computeForcesKernel = buildKernel(device, "kernels/computeForces.cl", "computeForces");
}

cl_kernel QuadTree::buildKernel(cl_device_id device, const char* sourceFile, const char* functionName) {
    std::ifstream file(sourceFile);
    std::string sourceString((std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>());
    const char* source = sourceString.c_str();

    cl_program program = clCreateProgramWithSource(_context, 1, (const char**)&source, nullptr, nullptr);
    cl_int status = clBuildProgram(program, 0, nullptr, nullptr, nullptr, nullptr);
    checkError(status, program, device);
    return clCreateKernel(program, functionName, nullptr);
}

void QuadTree::checkError(cl_int status, cl_program program, cl_device_id device) {
    if (status != CL_SUCCESS) {
        size_t logSize = 0;
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, nullptr, &logSize);
        std::string log(logSize, '\0');
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, logSize, log.data(), nullptr);
        std::cerr << log << std::endl;
        abort();
    }
}
#endif

void QuadTree::computeBounds() {
    _nodes.clear();
    _nodes.push_back(QuadTreeNode());

    float left = std::numeric_limits<float>::max();
    float right = std::numeric_limits<float>::lowest();
    float top = std::numeric_limits<float>::max();
    float bottom = std::numeric_limits<float>::lowest();

    const int n = static_cast<int>(_particles.size());
    const int parallel = (_backend == ComputeBackend::OpenMP) ? 1 : 0;

#pragma omp parallel for reduction(min:left,top) reduction(max:right,bottom) if(parallel)
    for (int i = 0; i < n; ++i) {
        const float2 p = _particles[i].position;
        left = std::min(p.x, left);
        right = std::max(p.x, right);
        top = std::min(p.y, top);
        bottom = std::max(p.y, bottom);
    }

    _nodes[0].position.x = left;
    _nodes[0].position.y = top;
    _nodes[0].dimentions.x = right - left + ROUNDING_ERROR_CORRECTION;
    _nodes[0].dimentions.y = bottom - top + ROUNDING_ERROR_CORRECTION;
}

void QuadTree::build() {
    // while particles insert
    for (int i = 0; i < _particles.size(); i++){
        Particle& particle = _particles[i];
        if (!contains(&_nodes[0], &particle)) {
            continue;
        }

        // go down the tree
        int idx = 0;
        while (_nodes[idx].leavesIdx != -1) {
            int newIdx = idx;
            for (int i = 0; i < 4; i++) {
                unsigned int leafIdx = _nodes[idx].leavesIdx + i;
                if (contains(&_nodes[leafIdx], &particle)) {
                    newIdx = leafIdx;
                }
            }
            idx = newIdx;
        }

        // split if needed
        while (_nodes[idx].particleIdx != -1) {
            _nodes[idx].leavesIdx = _nodes.size();
            
            int particleIdx = _nodes[idx].particleIdx;
            Particle* p = &_particles[particleIdx];
            
            if (p->position.x == particle.position.x && p->position.y == particle.position.y) {
                std::cout << "two particles with the exact same position";
                return;
            }
            
            _nodes[idx].particleIdx = -1;

            for (int i = 0; i < 4; i++) {
                _nodes.push_back(QuadTreeNode());
                int newIdx = _nodes.size() - 1;
                _nodes[newIdx].dimentions = divide(_nodes[idx].dimentions, 2.f);
                switch (i) {
                case 0:
                    _nodes[newIdx].position = _nodes[idx].position;
                    break;
                case 1:
                    _nodes[newIdx].position = { _nodes[idx].position.x + _nodes[newIdx].dimentions.x, _nodes[idx].position.y };
                    break;
                case 2:
                    _nodes[newIdx].position = { _nodes[idx].position.x, _nodes[idx].position.y + _nodes[newIdx].dimentions.y};
                    break;
                default:
                    _nodes[newIdx].position = { _nodes[idx].position.x + _nodes[newIdx].dimentions.x, _nodes[idx].position.y + _nodes[newIdx].dimentions.y };
                }
                if (contains(&_nodes[newIdx], p)) {
                    _nodes[newIdx].particleIdx = particleIdx;
                }
            }
            // go to correct quad
            for (int i = 0; i < 4; i++) {
                int leafIdx = _nodes[idx].leavesIdx + i;
                if (contains(&_nodes[leafIdx], &particle)) {
                    idx = leafIdx;
                    break;
                }
            }
        }
        // insert
        _nodes[idx].particleIdx = i;
    }

#ifdef QUADTREES_ENABLE_OPENCL
    if (_backend == ComputeBackend::OpenCL) {
        if (_nodeBuffer != nullptr) {
            clReleaseMemObject(_nodeBuffer);
        }
        _nodeBuffer = clCreateBuffer(_context, CL_MEM_READ_WRITE, _nodes.size() * sizeof(QuadTreeNode), nullptr, nullptr);
    }
#endif
}

void QuadTree::calculateMass() {
    for (int i = _nodes.size() - 1; i >= 0; i--) {
        if (_nodes[i].particleIdx != -1) {
            _nodes[i].mass = _particles[_nodes[i].particleIdx].mass;
            _nodes[i].massCenter = _particles[_nodes[i].particleIdx].position;
        }
        else if (_nodes[i].leavesIdx == -1) {
            _nodes[i].mass = 0;
            _nodes[i].massCenter = { 0,0 };
        }
        else {
            _nodes[i].massCenter = _nodes[_nodes[i].leavesIdx].massCenter;
            _nodes[i].mass = _nodes[_nodes[i].leavesIdx].mass;
            for (int j = 1; j <= 3; j++) {
                _nodes[i].massCenter = { _nodes[i].massCenter.x * _nodes[i].mass +
                    _nodes[_nodes[i].leavesIdx + j].massCenter.x * _nodes[_nodes[i].leavesIdx + j].mass,
                    _nodes[i].massCenter.y * _nodes[i].mass +
                    _nodes[_nodes[i].leavesIdx + j].massCenter.y * _nodes[_nodes[i].leavesIdx + j].mass };
                _nodes[i].mass += _nodes[_nodes[i].leavesIdx + j].mass;
                if (_nodes[i].mass != 0) {
                    _nodes[i].massCenter.x /= _nodes[i].mass;
                    _nodes[i].massCenter.y /= _nodes[i].mass;
                }
            }
        }
    }
}

float2 QuadTree::totalForceOn(int particleIdx) const {
    float2 totalForce{ 0.f, 0.f };
    int stack[MAX_STACK];
    int top = -1;
    stack[++top] = 0;
    const float2 p = _particles[particleIdx].position;
    const float pMass = _particles[particleIdx].mass;

    while (top > -1) {
        const int i = stack[top--];

        if (_nodes[i].particleIdx == particleIdx) {
            continue;
        }
        if (_nodes[i].particleIdx != -1) {
            const Particle& other = _particles[_nodes[i].particleIdx];
            const float2 r = distance(p, other.position);
            float distance_length = std::max(length(r), MIN);
            const float2 unit_vector = unit(r);
            const float mag = other.mass * pMass / (distance_length * distance_length);
            totalForce.x += mag * unit_vector.x;
            totalForce.y += mag * unit_vector.y;
            continue;
        }

        const float s = _nodes[i].dimentions.x;
        const float2 rCenter = distance(p, _nodes[i].massCenter);
        const float d = length(rCenter);
        if (d > 0.f && (s / d) < THETA) {
            float distance_length = std::max(d, MIN);
            const float2 unit_vector = unit(rCenter);
            const float mag = _nodes[i].mass * pMass / (distance_length * distance_length);
            totalForce.x += mag * unit_vector.x;
            totalForce.y += mag * unit_vector.y;
            continue;
        }
        else if (_nodes[i].leavesIdx != -1 && top + 4 < MAX_STACK) {
            stack[++top] = _nodes[i].leavesIdx;
            stack[++top] = _nodes[i].leavesIdx + 1;
            stack[++top] = _nodes[i].leavesIdx + 2;
            stack[++top] = _nodes[i].leavesIdx + 3;
        }
    }

    return totalForce;
}

void QuadTree::computeForcesCpu() {
    const int n = static_cast<int>(_particles.size());
    const int parallel = (_backend == ComputeBackend::OpenMP) ? 1 : 0;

    // Barnes-Hut cost varies with particle location; dynamic chunks keep cores busy.
#pragma omp parallel for schedule(dynamic, 32) if(parallel)
    for (int id = 0; id < n; ++id) {
        const float2 force = totalForceOn(id);
        updateAcceletation(&_particles[id], force);
    }
}

void QuadTree::computeForces() {
#ifdef QUADTREES_ENABLE_OPENCL
    if (_backend == ComputeBackend::OpenCL) {
        clEnqueueWriteBuffer(_queue, _particleBuffer, CL_TRUE, 0, _particles.size() * sizeof(Particle), _particles.data(), 0, nullptr, nullptr);
        clEnqueueWriteBuffer(_queue, _nodeBuffer, CL_TRUE, 0, _nodes.size() * sizeof(QuadTreeNode), _nodes.data(), 0, nullptr, nullptr);

        clSetKernelArg(_computeForcesKernel, 0, sizeof(cl_mem), &_particleBuffer);
        clSetKernelArg(_computeForcesKernel, 1, sizeof(cl_mem), &_nodeBuffer);

        clEnqueueNDRangeKernel(_queue, _computeForcesKernel, 1, nullptr, &_globalDimensions, nullptr, 0, nullptr, nullptr);
        clFinish(_queue);
        return;
    }
#endif
    computeForcesCpu();
}

void QuadTree::updatePositionsCpu(float dt) {
    const int n = static_cast<int>(_particles.size());
    const int parallel = (_backend == ComputeBackend::OpenMP) ? 1 : 0;

#pragma omp parallel for schedule(static) if(parallel)
    for (int i = 0; i < n; ++i) {
        move(&_particles[i], dt);
    }
}

void QuadTree::updatePositions(float dt) {
#ifdef QUADTREES_ENABLE_OPENCL
    if (_backend == ComputeBackend::OpenCL) {
        clSetKernelArg(_updatePositionsKernel, 0, sizeof(cl_mem), &_particleBuffer);
        clSetKernelArg(_updatePositionsKernel, 1, sizeof(float), &dt);
        clEnqueueNDRangeKernel(_queue, _updatePositionsKernel, 1, nullptr, &_globalDimensions, nullptr, 0, nullptr, nullptr);
        clEnqueueReadBuffer(_queue, _particleBuffer, CL_TRUE, 0, _particles.size() * sizeof(Particle), _particles.data(), 0, nullptr, nullptr);
        clFinish(_queue);
        return;
    }
#endif
    updatePositionsCpu(dt);
}

void QuadTree::step(float dt) {
    computeBounds();
    build();
    calculateMass();
    computeForces();
    updatePositions(dt);
}

void QuadTree::draw(sf::RenderWindow& window, bool transform, bool showGrid, bool showMassCenter) {
    float2 scale;
    float2 translation;
    float particleScale;
    if (transform) {
        scale = { window.getSize().x / _nodes[0].dimentions.x, window.getSize().y / _nodes[0].dimentions.y };
        translation = { -_nodes[0].position.x * scale.x , -_nodes[0].position.y * scale.y};
        particleScale = 1 / _nodes[0].dimentions.x;
    }
    else {
        scale = { window.getSize().x / 1.f, window.getSize().y / 1.f };
        translation = { 0.f * scale.x , 0.f * scale.y };
        particleScale = 1;
    }
    
    if (showGrid) {
        for (QuadTreeNode& node : _nodes) {
            sf::RectangleShape r(sf::Vector2f(node.dimentions.x * scale.x, node.dimentions.y * scale.y));
            r.setPosition(sf::Vector2f(node.position.x * scale.x + translation.x, node.position.y * scale.y + translation.y));
            r.setOutlineThickness(1.f);
            r.setFillColor(sf::Color::Black);
            r.setOutlineColor(sf::Color(50, 50, 50));
            window.draw(r);
        }
    }
    
    for (Particle& particle : _particles) {
        double radius = std::max(particle.mass * 5.f * particleScale, 1.f);
        sf::CircleShape c(radius);
        c.setPosition(sf::Vector2f(particle.position.x * scale.x - radius + translation.x, particle.position.y * scale.y - radius + translation.y));
        c.setFillColor(sf::Color::White);
        window.draw(c);
    }

    if (showMassCenter) {
        int radius = 5;
        sf::CircleShape c(radius);
        c.setPosition(sf::Vector2f(_nodes[0].massCenter.x * scale.x - radius + translation.x, _nodes[0].massCenter.y * scale.y - radius + translation.y));
        c.setFillColor(sf::Color::Red);
        window.draw(c);
    }
}
