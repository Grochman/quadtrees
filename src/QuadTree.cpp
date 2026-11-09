#include "QuadTree.h"
#include "Fail.h"
#include <CL/cl.h>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

#define ROUNDING_ERROR_CORRECTION 0.0001f
#define MIN_CELL_SIZE 1e-8f

namespace {
constexpr char kUpdatePositions[] = {
#embed "kernels/updatePositions.cl"
    , '\0'};
constexpr char kComputeForces[] = {
#embed "kernels/computeForces.cl"
    , '\0'};
} // namespace

QuadTree::QuadTree(std::vector<Particle> &particles)
    : _particles(particles), _context(nullptr), _queue(nullptr),
      _particleBuffer(nullptr), _nodeBuffer(nullptr),
      _updatePositionsKernel(nullptr), _computeForcesKernel(nullptr) {

    if (_particles.empty()) {
        fatal("cannot simulate an empty particle set");
    }

    cl_int status = CL_SUCCESS;
    cl_uint platformCount = 0;
    status = clGetPlatformIDs(0, nullptr, &platformCount);
    checkError(status, "clGetPlatformIDs count");
    if (platformCount == 0) {
        fatal("no OpenCL platform found");
    }

    cl_platform_id platform = nullptr;
    status = clGetPlatformIDs(1, &platform, nullptr);
    checkError(status, "clGetPlatformIDs");

    cl_device_id device = nullptr;
    status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, nullptr);
    checkError(status, "clGetDeviceIDs GPU");

    _context = clCreateContext(nullptr, 1, &device, nullptr, nullptr, &status);
    checkError(status, "clCreateContext");
    _queue =
        clCreateCommandQueueWithProperties(_context, device, nullptr, &status);
    checkError(status, "clCreateCommandQueueWithProperties");

    _particleBuffer =
        clCreateBuffer(_context, CL_MEM_READ_WRITE,
                       _particles.size() * sizeof(Particle), nullptr, &status);
    checkError(status, "clCreateBuffer particles");
    _globalDimensions = _particles.size();

    _updatePositionsKernel =
        buildKernel(device, kUpdatePositions, "updatePositions");
    _computeForcesKernel = buildKernel(device, kComputeForces, "computeForces");
}

cl_kernel QuadTree::buildKernel(cl_device_id device, const char *source,
                                const char *functionName) {

    cl_int status = CL_SUCCESS;
    cl_program program = clCreateProgramWithSource(
        _context, 1, (const char **)&source, nullptr, &status);
    checkError(status, "clCreateProgramWithSource");
    status = clBuildProgram(program, 1, &device, nullptr, nullptr, nullptr);
    checkBuildError(status, program, device);

    cl_kernel kernel = clCreateKernel(program, functionName, &status);
    checkError(status, "clCreateKernel");
    return kernel;
}

void QuadTree::checkError(cl_int status, const char *what) {
    if (status != CL_SUCCESS) {
        std::cerr << "OpenCL error " << status << " in " << what << std::endl;
        abort();
    }
}

void QuadTree::checkBuildError(cl_int status, cl_program program,
                               cl_device_id device) {
    if (status != CL_SUCCESS) {
        size_t logSize = 0;
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, nullptr,
                              &logSize);
        std::string log(logSize, '\0');
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, logSize,
                              log.data(), nullptr);
        std::cerr << log << std::endl;
        abort();
    }
}

void QuadTree::computeBounds() {
    _nodes.clear();
    _nodes.push_back(QuadTreeNode());

    float left = _particles[0].position.x;
    float right = _particles[0].position.x;
    float top = _particles[0].position.y;
    float bottom = _particles[0].position.y;

    for (Particle &particle : _particles) {
        if (!std::isfinite(particle.position.x) ||
            !std::isfinite(particle.position.y)) {
            fatal("particle position is not finite");
        }
        left = std::min(particle.position.x, left);
        right = std::max(particle.position.x, right);
        top = std::min(particle.position.y, top);
        bottom = std::max(particle.position.y, bottom);
    }

    _nodes[0].position.x = left;
    _nodes[0].position.y = top;
    _nodes[0].dimentions.x = right - left + ROUNDING_ERROR_CORRECTION;
    _nodes[0].dimentions.y = bottom - top + ROUNDING_ERROR_CORRECTION;
}

void QuadTree::build() {
    // insert each particle into a leaf
    for (int i = 0; i < static_cast<int>(_particles.size()); i++) {
        Particle &particle = _particles[i];
        if (!contains(&_nodes[0], &particle)) {
            fatal("particle is outside the root bounding box");
        }

        // go down the tree to a leaf (midpoint picks exactly one child)
        int idx = 0;
        while (_nodes[idx].leavesIdx != -1) {
            idx = _nodes[idx].leavesIdx + childIndex(&_nodes[idx], &particle);
        }

        // split while this leaf already holds another particle
        while (_nodes[idx].particleIdx != -1) {
            if (_nodes[idx].dimentions.x < MIN_CELL_SIZE ||
                _nodes[idx].dimentions.y < MIN_CELL_SIZE) {
                fatal("quadtree cell underflow; particles are too close to "
                      "split");
            }

            int particleIdx = _nodes[idx].particleIdx;
            Particle *p = &_particles[particleIdx];

            if (p->position.x == particle.position.x &&
                p->position.y == particle.position.y) {
                fatal("two particles have the exact same position");
            }

            // four children are appended as a contiguous block
            const int leavesIdx = static_cast<int>(_nodes.size());
            _nodes[idx].leavesIdx = leavesIdx;
            _nodes[idx].particleIdx = -1;

            for (int child = 0; child < 4; child++) {
                _nodes.push_back(QuadTreeNode());
                int newIdx = static_cast<int>(_nodes.size()) - 1;
                _nodes[newIdx].dimentions = divide(_nodes[idx].dimentions, 2.f);
                switch (child) {
                case 0: // top-left
                    _nodes[newIdx].position = _nodes[idx].position;
                    break;
                case 1: // top-right
                    _nodes[newIdx].position = {_nodes[idx].position.x +
                                                   _nodes[newIdx].dimentions.x,
                                               _nodes[idx].position.y};
                    break;
                case 2: // bottom-left
                    _nodes[newIdx].position = {_nodes[idx].position.x,
                                               _nodes[idx].position.y +
                                                   _nodes[newIdx].dimentions.y};
                    break;
                default: // bottom-right
                    _nodes[newIdx].position = {
                        _nodes[idx].position.x + _nodes[newIdx].dimentions.x,
                        _nodes[idx].position.y + _nodes[newIdx].dimentions.y};
                }
            }

            // move the old occupant into its child, then follow the new
            // particle
            const int oldChild = leavesIdx + childIndex(&_nodes[idx], p);
            _nodes[oldChild].particleIdx = particleIdx;
            idx = leavesIdx + childIndex(&_nodes[idx], &particle);
        }

        if (_nodes[idx].leavesIdx != -1) {
            fatal("attempted to store a particle in an internal node");
        }
        // insert
        _nodes[idx].particleIdx = i;
    }

    if (_nodeBuffer != nullptr) {
        checkError(clReleaseMemObject(_nodeBuffer), "clReleaseMemObject nodes");
        _nodeBuffer = nullptr;
    }
    cl_int status = CL_SUCCESS;
    _nodeBuffer =
        clCreateBuffer(_context, CL_MEM_READ_WRITE,
                       _nodes.size() * sizeof(QuadTreeNode), nullptr, &status);
    checkError(status, "clCreateBuffer nodes");
}

void QuadTree::calculateMass() {
    for (int i = static_cast<int>(_nodes.size()) - 1; i >= 0; i--) {
        if (_nodes[i].particleIdx != -1) {
            _nodes[i].mass = _particles[_nodes[i].particleIdx].mass;
            _nodes[i].massCenter = _particles[_nodes[i].particleIdx].position;
        } else if (_nodes[i].leavesIdx == -1) {
            _nodes[i].mass = 0;
            _nodes[i].massCenter = {0, 0};
        } else {
            _nodes[i].mass = 0;
            _nodes[i].massCenter = {0, 0};
            for (int j = 0; j < 4; j++) {
                const QuadTreeNode &child = _nodes[_nodes[i].leavesIdx + j];
                if (child.mass == 0.f) {
                    continue;
                }
                _nodes[i].massCenter.x += child.massCenter.x * child.mass;
                _nodes[i].massCenter.y += child.massCenter.y * child.mass;
                _nodes[i].mass += child.mass;
            }
            if (_nodes[i].mass != 0.f) {
                _nodes[i].massCenter.x /= _nodes[i].mass;
                _nodes[i].massCenter.y /= _nodes[i].mass;
            }
        }
    }
}

void QuadTree::computeForces() {
    if (_nodeBuffer == nullptr) {
        fatal("force kernel ran before the tree buffer was created");
    }
    checkError(clEnqueueWriteBuffer(_queue, _particleBuffer, CL_TRUE, 0,
                                    _particles.size() * sizeof(Particle),
                                    _particles.data(), 0, nullptr, nullptr),
               "clEnqueueWriteBuffer particles");
    checkError(clEnqueueWriteBuffer(_queue, _nodeBuffer, CL_TRUE, 0,
                                    _nodes.size() * sizeof(QuadTreeNode),
                                    _nodes.data(), 0, nullptr, nullptr),
               "clEnqueueWriteBuffer nodes");

    checkError(clSetKernelArg(_computeForcesKernel, 0, sizeof(cl_mem),
                              &_particleBuffer),
               "clSetKernelArg forces particles");
    checkError(
        clSetKernelArg(_computeForcesKernel, 1, sizeof(cl_mem), &_nodeBuffer),
        "clSetKernelArg forces nodes");

    checkError(clEnqueueNDRangeKernel(_queue, _computeForcesKernel, 1, nullptr,
                                      &_globalDimensions, nullptr, 0, nullptr,
                                      nullptr),
               "clEnqueueNDRangeKernel computeForces");
    checkError(clFinish(_queue), "clFinish computeForces");
}

void QuadTree::updatePositions(float dt) {
    if (!std::isfinite(dt) || dt < 0.f) {
        fatal("non-finite or negative integration step");
    }
    if (dt == 0.f) {
        warn("skipped position update because dt is zero");
        return;
    }
    checkError(clSetKernelArg(_updatePositionsKernel, 0, sizeof(cl_mem),
                              &_particleBuffer),
               "clSetKernelArg update particles");
    checkError(clSetKernelArg(_updatePositionsKernel, 1, sizeof(float), &dt),
               "clSetKernelArg update dt");
    checkError(clEnqueueNDRangeKernel(_queue, _updatePositionsKernel, 1,
                                      nullptr, &_globalDimensions, nullptr, 0,
                                      nullptr, nullptr),
               "clEnqueueNDRangeKernel updatePositions");
    checkError(clEnqueueReadBuffer(_queue, _particleBuffer, CL_TRUE, 0,
                                   _particles.size() * sizeof(Particle),
                                   _particles.data(), 0, nullptr, nullptr),
               "clEnqueueReadBuffer particles");
    checkError(clFinish(_queue), "clFinish updatePositions");
}

void QuadTree::draw(sf::RenderWindow &window, bool transform, bool showGrid,
                    bool showMassCenter) {
    float2 scale;
    float2 translation;
    float particleScale;
    if (transform) {
        if (_nodes[0].dimentions.x <= 0.f || _nodes[0].dimentions.y <= 0.f) {
            warn("skipped view transform because the root cell has "
                 "non-positive size");
            transform = false;
        }
    }
    if (transform) {
        scale = {window.getSize().x / _nodes[0].dimentions.x,
                 window.getSize().y / _nodes[0].dimentions.y};
        translation = {-_nodes[0].position.x * scale.x,
                       -_nodes[0].position.y * scale.y};
        particleScale = 1 / _nodes[0].dimentions.x;
    } else {
        scale = {window.getSize().x / 1.f, window.getSize().y / 1.f};
        translation = {0.f * scale.x, 0.f * scale.y};
        particleScale = 1;
    }

    if (showGrid) {
        for (QuadTreeNode &node : _nodes) {
            sf::RectangleShape r(sf::Vector2f(node.dimentions.x * scale.x,
                                              node.dimentions.y * scale.y));
            r.setPosition(
                sf::Vector2f(node.position.x * scale.x + translation.x,
                             node.position.y * scale.y + translation.y));
            r.setOutlineThickness(1.f);
            r.setFillColor(sf::Color::Black);
            r.setOutlineColor(sf::Color(50, 50, 50));
            window.draw(r);
        }
    }

    for (Particle &particle : _particles) {
        double radius = std::max(particle.mass * 5.f * particleScale, 1.f);
        sf::CircleShape c(radius);
        c.setPosition(sf::Vector2f(
            particle.position.x * scale.x - radius + translation.x,
            particle.position.y * scale.y - radius + translation.y));
        c.setFillColor(sf::Color::White);
        window.draw(c);
    }

    if (showMassCenter) {
        int radius = 5;
        sf::CircleShape c(radius);
        c.setPosition(sf::Vector2f(
            _nodes[0].massCenter.x * scale.x - radius + translation.x,
            _nodes[0].massCenter.y * scale.y - radius + translation.y));
        c.setFillColor(sf::Color::Red);
        window.draw(c);
    }
}

bool QuadTree::contains(const QuadTreeNode *node, const Particle *particle) {
    if (particle->position.x >= node->position.x &&
        particle->position.y >= node->position.y &&
        particle->position.x <= node->position.x + node->dimentions.x &&
        particle->position.y <= node->position.y + node->dimentions.y) {
        return true;
    }
    return false;
}

int QuadTree::childIndex(const QuadTreeNode *node, const Particle *particle) {
    if (!contains(node, particle)) {
        fatal("particle is not inside the node used for child selection");
    }
    const float midX = node->position.x + node->dimentions.x * 0.5f;
    const float midY = node->position.y + node->dimentions.y * 0.5f;
    int index = 0;
    if (particle->position.x >= midX) {
        index += 1;
    }
    if (particle->position.y >= midY) {
        index += 2;
    }
    return index;
}
