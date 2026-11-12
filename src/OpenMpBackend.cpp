#include "OpenMpBackend.h"
#include "Fail.h"

#include <cmath>
#include <omp.h>

#define MAX_STACK 64
#define MIN_DIST 0.1f
#define THETA 0.5f

OpenMpBackend::OpenMpBackend(std::vector<Particle> &particles,
                             std::vector<QuadTreeNode> &nodes)
    : ComputeBackend(particles, nodes) {}

float2 OpenMpBackend::calculateForce(float2 distance, float mass,
                                     const Particle &particle) {
    const float distanceLength =
        std::max(std::hypot(distance.x, distance.y), MIN_DIST);
    const float2 unitVector = {distance.x / distanceLength,
                               distance.y / distanceLength};
    const float scale =
        mass * particle.mass / (distanceLength * distanceLength);
    return {scale * unitVector.x, scale * unitVector.y};
}

float2 OpenMpBackend::totalForce(int particleIdx) {
    float2 force = {0.f, 0.f};
    int stack[MAX_STACK];
    int top = -1;
    stack[++top] = 0;
    while (top > -1) {
        const int i = stack[top--];

        if (_nodes[i].particleIdx == particleIdx) {
            continue;
        }
        if (_nodes[i].particleIdx != -1) {
            const Particle &other = _particles[_nodes[i].particleIdx];
            const float2 r = {other.position.x -
                                  _particles[particleIdx].position.x,
                              other.position.y -
                                  _particles[particleIdx].position.y};
            const float2 f =
                calculateForce(r, other.mass, _particles[particleIdx]);
            force.x += f.x;
            force.y += f.y;
            continue;
        }
        const float s = _nodes[i].dimensions.x;
        const float dx =
            _nodes[i].massCenter.x - _particles[particleIdx].position.x;
        const float dy =
            _nodes[i].massCenter.y - _particles[particleIdx].position.y;
        const float d = std::hypot(dx, dy);
        if (d > 0.f && s / d < THETA) {
            const float2 r = {dx, dy};
            const float2 f =
                calculateForce(r, _nodes[i].mass, _particles[particleIdx]);
            force.x += f.x;
            force.y += f.y;
            continue;
        }
        if (_nodes[i].leavesIdx != -1) {
            if (top + 4 >= MAX_STACK) {
                continue;
            }
            stack[++top] = _nodes[i].leavesIdx;
            stack[++top] = _nodes[i].leavesIdx + 1;
            stack[++top] = _nodes[i].leavesIdx + 2;
            stack[++top] = _nodes[i].leavesIdx + 3;
        }
    }
    return force;
}

void OpenMpBackend::computeBounds() {
    _nodes.clear();
    _nodes.push_back(QuadTreeNode());
    if (_particles.empty()) {
        return;
    }

    for (const Particle &particle : _particles) {
        if (!std::isfinite(particle.position.x) ||
            !std::isfinite(particle.position.y)) {
            fatal("particle position is not finite");
        }
    }

    float left = _particles[0].position.x;
    float right = _particles[0].position.x;
    float top = _particles[0].position.y;
    float bottom = _particles[0].position.y;
    const int n = static_cast<int>(_particles.size());

#pragma omp parallel for reduction(min : left, top) reduction(max : right, bottom)
    for (int i = 0; i < n; i++) {
        left = std::min(left, _particles[i].position.x);
        right = std::max(right, _particles[i].position.x);
        top = std::min(top, _particles[i].position.y);
        bottom = std::max(bottom, _particles[i].position.y);
    }

    _nodes[0].position.x = left;
    _nodes[0].position.y = top;
    _nodes[0].dimensions.x = right - left + 0.0001f;
    _nodes[0].dimensions.y = bottom - top + 0.0001f;
}

void OpenMpBackend::computeForces() {
    const int n = static_cast<int>(_particles.size());
#pragma omp parallel for
    for (int i = 0; i < n; i++) {
        const float2 force = totalForce(i);
        if (!(_particles[i].mass > 0.f)) {
            continue;
        }
        _particles[i].acceleration.x += force.x / _particles[i].mass;
        _particles[i].acceleration.y += force.y / _particles[i].mass;
    }
}

void OpenMpBackend::updatePositions(float dt) {
    if (!std::isfinite(dt) || dt < 0.f) {
        fatal("non-finite or negative integration step");
    }
    if (dt == 0.f) {
        warn("skipped position update because dt is zero");
        return;
    }

    const int n = static_cast<int>(_particles.size());
#pragma omp parallel for
    for (int i = 0; i < n; i++) {
        Particle &p = _particles[i];
        p.velocity.x += p.acceleration.x * dt * 0.5f;
        p.velocity.y += p.acceleration.y * dt * 0.5f;
        p.position.x += p.velocity.x * dt;
        p.position.y += p.velocity.y * dt;
        p.velocity.x += p.acceleration.x * dt * 0.5f;
        p.velocity.y += p.acceleration.y * dt * 0.5f;
        p.acceleration.x = 0.f;
        p.acceleration.y = 0.f;
    }
}
