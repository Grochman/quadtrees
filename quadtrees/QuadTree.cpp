#include "QuadTree.h"
#include <CL/cl.h>
#include <iostream>
#include <stack>
#include <fstream>

#define MIN 0.1f
#define ROUNDING_ERROR_CORRECTION 0.0001f

QuadTree::QuadTree(std::vector<Particle>& particles) 
    : _particles(particles){
    cl_platform_id platfmors;
    clGetPlatformIDs(1, &platfmors, nullptr);
    cl_device_id device;
    clGetDeviceIDs(platfmors, CL_DEVICE_TYPE_GPU, 1, &device, nullptr);

    
    _context = clCreateContext(nullptr, 1, &device, nullptr, nullptr, nullptr);
    _queue = clCreateCommandQueueWithProperties(_context, device, nullptr, nullptr);
    
    _buffer = clCreateBuffer(_context, CL_MEM_READ_WRITE, _particles.size() * sizeof(Particle), nullptr, nullptr);
    _globalDimensions = _particles.size();
    std::ifstream file("kernels/updatePositions.cl");
    std::string sourceString((std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>());
    const char* source = sourceString.c_str();
    
    cl_program program = clCreateProgramWithSource(_context, 1, (const char**)&source, nullptr, nullptr);
    cl_int status = clBuildProgram(program, 0, nullptr, nullptr, nullptr, nullptr);
    if (status != CL_SUCCESS) {
        size_t logSize = 0;
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, nullptr, &logSize);
        std::string log(logSize, '\0');
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, logSize, log.data(), nullptr);
        std::cerr << log << std::endl;
        abort();
    }
    _updatePositionsKernel = clCreateKernel(program, "updatePositions", nullptr);
};

float2 QuadTree::calculateForce(float2& distance, const float mass, Particle* particle) {
    float2 unit_vector = unit(distance);
    float distance_length = length(distance);
    distance_length = std::max(distance_length, MIN);
    return { (mass * particle->mass / pow(distance_length, 2.f)) * unit_vector.x, (mass * particle->mass / pow(distance_length, 2.f)) * unit_vector.y };
}

void QuadTree::computeBounds() {
    _nodes.clear();
    _nodes.push_back(QuadTreeNode());

    float left = INT_MAX;
    float right = INT_MIN;
    float top = INT_MAX;
    float bottom = INT_MIN;

    for (Particle& particle : _particles) {
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
    // while particles insert
    for (int i = 0; i < _particles.size(); i++){
        Particle& particle = _particles[i];
        if (!contains(&_nodes[0], &particle)) {
            continue;
        }

        // go down the tree
        int idx = 0;
        while (_nodes[idx].leavesIdx != -1) {
            int newIdx;
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

float2 QuadTree::getTotalForce(int particleIdx) {
    float2 totalForce = { 0,0 };

    std::stack<int> idxs;
    idxs.push(0);

    while (!idxs.empty()) {
        int i = idxs.top();
        idxs.pop();

        if (_nodes[i].particleIdx == particleIdx) {
            continue;
        }
        if (_nodes[i].particleIdx != -1) {
            float2 r = distance(_particles[particleIdx].position, _particles[_nodes[i].particleIdx].position);
            totalForce = sum(totalForce, calculateForce(r, _particles[_nodes[i].particleIdx].mass, &_particles[particleIdx]));
            continue;
        }
        const float s = _nodes[i].dimentions.x;
        const float d = length(distance(_particles[particleIdx].position, _nodes[i].massCenter));
        if (s / d < 0.5) {
            float2 r = distance(_particles[particleIdx].position, _nodes[i].massCenter);
            totalForce = sum(totalForce, calculateForce(r, _nodes[i].mass, &_particles[particleIdx]));
            continue;
        }
        else if (_nodes[i].leavesIdx != -1) {
            idxs.push(_nodes[i].leavesIdx);
            idxs.push(_nodes[i].leavesIdx + 1);
            idxs.push(_nodes[i].leavesIdx + 2);
            idxs.push(_nodes[i].leavesIdx + 3);
        }
    }
    
    return totalForce;
}

void QuadTree::computeForces() {
    for (int i = 0; i < _particles.size(); i++) {
        float2 force = getTotalForce(i);
        updateAcceletation(&_particles[i], force);
    }
}

void QuadTree::updatePositions(float dt) {
    clEnqueueWriteBuffer(_queue, _buffer, CL_TRUE, 0, _particles.size() * sizeof(Particle), _particles.data(), 0, nullptr, nullptr);
    clSetKernelArg(_updatePositionsKernel, 0, sizeof(cl_mem), &_buffer);
    clSetKernelArg(_updatePositionsKernel, 1, sizeof(float), &dt);
    clEnqueueNDRangeKernel(_queue, _updatePositionsKernel, 1, nullptr, &_globalDimensions, nullptr, 0, nullptr, nullptr);
    clEnqueueReadBuffer(_queue, _buffer, CL_TRUE, 0, _particles.size() * sizeof(Particle), _particles.data(), 0, nullptr, nullptr);
    clFinish(_queue);
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