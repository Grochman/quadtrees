#include "OpenMpBackend.h"
#include "Fail.h"
#include "Vector2d.h"

#include <cmath>
#include <omp.h>

#define MAX_STACK 64
#define MIN_DIST 0.1f
#define THETA 0.5f
#define MIN_CELL_SIZE 1e-8f
#define BUILD_NODE_CAP_PER_PARTICLE 64
#define BUILD_TASK_GRAIN 64

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
            const float2 r = {
                other.position.x - _particles[particleIdx].position.x,
                other.position.y - _particles[particleIdx].position.y};
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

#pragma omp parallel for reduction(min : left, top)                            \
    reduction(max : right, bottom)
    for (int i = 1; i < n; i++) {
        left = std::min(left, _particles[i].position.x);
        right = std::max(right, _particles[i].position.x);
        top = std::min(top, _particles[i].position.y);
        bottom = std::max(bottom, _particles[i].position.y);
    }

    _nodes[0].position.x = left;
    _nodes[0].position.y = top;
    _nodes[0].dimensions.x = right - left;
    _nodes[0].dimensions.y = bottom - top;
}

int OpenMpBackend::allocChildren() {
    int first;
    // get the next free node index and increment atomically
#pragma omp atomic capture
    first = _nextNode += 4;
    first -= 4;
    if (first + 4 > static_cast<int>(_nodes.size())) {
        fatal("quadtree node pool overflow");
    }
    return first;
}

void OpenMpBackend::initChildBoxes(int parentIdx, int firstChild) {
    const QuadTreeNode &parent = _nodes[parentIdx];
    const float2 half = divide(parent.dimensions, 2.f);
    for (int child = 0; child < 4; child++) {
        QuadTreeNode &node = _nodes[firstChild + child];
        node = QuadTreeNode();
        node.dimensions = half;
        switch (child) {
        case 0:
            node.position = parent.position;
            break;
        case 1:
            node.position = {parent.position.x + half.x, parent.position.y};
            break;
        case 2:
            node.position = {parent.position.x, parent.position.y + half.y};
            break;
        default:
            node.position = {parent.position.x + half.x,
                             parent.position.y + half.y};
        }
    }
}

void OpenMpBackend::partitionRange(int nodeIdx, int begin, int end,
                                   int splits[5]) {
    int counts[4] = {0, 0, 0, 0}; // count of particles in each quadrant
    for (int i = begin; i < end; i++) {
        counts[childIndex(&_nodes[nodeIdx], &_particles[_order[i]])]++;
    }
    splits[0] = begin;
    for (int q = 0; q < 4; q++) { // update splits to reflect the new ranges
        splits[q + 1] = splits[q] + counts[q];
    }
    int cursor[4] = {splits[0], splits[1], splits[2], splits[3]};
    // go over all, get quad, put in new position, increment cursor for scratch
    for (int i = begin; i < end; i++) {
        const int q = childIndex(&_nodes[nodeIdx], &_particles[_order[i]]);
        _orderScratch[cursor[q]++] = _order[i];
    }
    for (int i = begin; i < end; i++) {
        _order[i] = _orderScratch[i];
    }
}

void OpenMpBackend::buildSubtree(int nodeIdx, int begin, int end) {
    if (end - begin <= 0) { // empty node
        return;
    }
    if (end - begin == 1) { // single particle node
        _nodes[nodeIdx].particleIdx = _order[begin];
        return;
    }
    if (_nodes[nodeIdx].dimensions.x < MIN_CELL_SIZE ||
        _nodes[nodeIdx].dimensions.y < MIN_CELL_SIZE) {
        fatal("quadtree cell underflow; particles are too close to split");
    }

    const int firstChild = allocChildren();
    _nodes[nodeIdx].leavesIdx = firstChild;
    _nodes[nodeIdx].particleIdx = -1;
    initChildBoxes(nodeIdx, firstChild);

    int splits[5]; // index of begin and end of _order for each quadrant - child
                   // q owns _order[splits[q] .. _splits[q+1]]
    partitionRange(nodeIdx, begin, end, splits);

    for (int q = 0; q < 4; q++) {
        const int childBegin = splits[q];
        const int childEnd = splits[q + 1];
#pragma omp task firstprivate(                                                 \
        q, firstChild, childBegin,                                             \
            childEnd) if (childEnd - childBegin >                              \
                              BUILD_TASK_GRAIN) // do not spawn tasks if child
                                                // count under cap
        buildSubtree(firstChild + q, childBegin, childEnd);
    }
#pragma omp taskwait
}

void OpenMpBackend::build() {
    const int n = static_cast<int>(_particles.size());
    if (n == 0) {
        return;
    }

    _order.resize(static_cast<size_t>(n));
    _orderScratch.resize(static_cast<size_t>(n));
    for (int i = 0; i < n; i++) {
        _order[i] = i;
    }

    const int cap = n * BUILD_NODE_CAP_PER_PARTICLE + 1;
    if (static_cast<int>(_nodes.size()) < cap) {
        _nodes.resize(static_cast<size_t>(cap));
    }
    _nextNode = 1;

#pragma omp parallel
    {
#pragma omp single
        buildSubtree(0, 0, n);
    }

    _nodes.resize(static_cast<size_t>(_nextNode));
}

void OpenMpBackend::accumulateMass(int nodeIdx, int depth) {
    QuadTreeNode &node = _nodes[nodeIdx];
    if (node.particleIdx != -1) {
        node.mass = _particles[node.particleIdx].mass;
        node.massCenter = _particles[node.particleIdx].position;
        return;
    }
    if (node.leavesIdx == -1) {
        node.mass = 0;
        node.massCenter = {0, 0};
        return;
    }

    const int firstChildNodeIdx = node.leavesIdx;
    if (depth < 8) {
        for (int j = 0; j < 4; j++) {
#pragma omp task firstprivate(j)
            accumulateMass(firstChildNodeIdx + j, depth + 1);
        }
#pragma omp taskwait
    } else {
        for (int j = 0; j < 4; j++) {
            accumulateMass(firstChildNodeIdx + j, depth + 1);
        }
    }

    node.mass = 0;
    node.massCenter = {0, 0};
    for (int j = 0; j < 4; j++) {
        const QuadTreeNode &child = _nodes[firstChildNodeIdx + j];
        if (child.mass == 0.f) {
            continue;
        }
        node.massCenter.x += child.massCenter.x * child.mass;
        node.massCenter.y += child.massCenter.y * child.mass;
        node.mass += child.mass;
    }
    if (node.mass != 0.f) {
        node.massCenter.x /= node.mass;
        node.massCenter.y /= node.mass;
    }
}

void OpenMpBackend::calculateMass() {
    if (_nodes.empty()) {
        return;
    }
#pragma omp parallel
    {
#pragma omp single
        accumulateMass(0, 0);
    }
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
