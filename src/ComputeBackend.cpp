#include "ComputeBackend.h"
#include "Fail.h"
#include "OpenClBackend.h"
#include "OpenMpBackend.h"
#include "Vector2d.h"

#include <cmath>

#define ROUNDING_ERROR_CORRECTION 0.0001f
#define MIN_CELL_SIZE 1e-8f

ComputeBackend::ComputeBackend(std::vector<Particle> &particles)
    : _particles(particles) {}

void ComputeBackend::computeBounds(std::vector<QuadTreeNode> &nodes) {
    nodes.clear();
    nodes.push_back(QuadTreeNode());
    if (_particles.empty()) {
        return;
    }

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

    nodes[0].position.x = left;
    nodes[0].position.y = top;
    nodes[0].dimensions.x = right - left + ROUNDING_ERROR_CORRECTION;
    nodes[0].dimensions.y = bottom - top + ROUNDING_ERROR_CORRECTION;
}

void ComputeBackend::build(std::vector<QuadTreeNode> &nodes) {
    for (int i = 0; i < static_cast<int>(_particles.size()); i++) {
        Particle &particle = _particles[i];
        if (!contains(&nodes[0], &particle)) {
            fatal("particle is outside the root bounding box");
        }

        int idx = 0;
        while (nodes[idx].leavesIdx != -1) {
            idx = nodes[idx].leavesIdx + childIndex(&nodes[idx], &particle);
        }

        while (nodes[idx].particleIdx != -1) {
            if (nodes[idx].dimensions.x < MIN_CELL_SIZE ||
                nodes[idx].dimensions.y < MIN_CELL_SIZE) {
                fatal("quadtree cell underflow; particles are too close to "
                      "split");
            }

            int particleIdx = nodes[idx].particleIdx;
            Particle *p = &_particles[particleIdx];

            if (p->position.x == particle.position.x &&
                p->position.y == particle.position.y) {
                fatal("two particles have the exact same position");
            }

            const int leavesIdx = static_cast<int>(nodes.size());
            nodes[idx].leavesIdx = leavesIdx;
            nodes[idx].particleIdx = -1;

            for (int child = 0; child < 4; child++) {
                nodes.push_back(QuadTreeNode());
                int newIdx = static_cast<int>(nodes.size()) - 1;
                nodes[newIdx].dimensions = divide(nodes[idx].dimensions, 2.f);
                switch (child) {
                case 0:
                    nodes[newIdx].position = nodes[idx].position;
                    break;
                case 1:
                    nodes[newIdx].position = {nodes[idx].position.x +
                                                  nodes[newIdx].dimensions.x,
                                              nodes[idx].position.y};
                    break;
                case 2:
                    nodes[newIdx].position = {nodes[idx].position.x,
                                              nodes[idx].position.y +
                                                  nodes[newIdx].dimensions.y};
                    break;
                default:
                    nodes[newIdx].position = {
                        nodes[idx].position.x + nodes[newIdx].dimensions.x,
                        nodes[idx].position.y + nodes[newIdx].dimensions.y};
                }
            }

            const int oldChild = leavesIdx + childIndex(&nodes[idx], p);
            nodes[oldChild].particleIdx = particleIdx;
            idx = leavesIdx + childIndex(&nodes[idx], &particle);
        }

        if (nodes[idx].leavesIdx != -1) {
            fatal("attempted to store a particle in an internal node");
        }
        nodes[idx].particleIdx = i;
    }
}

void ComputeBackend::calculateMass(std::vector<QuadTreeNode> &nodes) {
    for (int i = static_cast<int>(nodes.size()) - 1; i >= 0; i--) {
        if (nodes[i].particleIdx != -1) {
            nodes[i].mass = _particles[nodes[i].particleIdx].mass;
            nodes[i].massCenter = _particles[nodes[i].particleIdx].position;
        } else if (nodes[i].leavesIdx == -1) {
            nodes[i].mass = 0;
            nodes[i].massCenter = {0, 0};
        } else {
            nodes[i].mass = 0;
            nodes[i].massCenter = {0, 0};
            for (int j = 0; j < 4; j++) {
                const QuadTreeNode &child = nodes[nodes[i].leavesIdx + j];
                if (child.mass == 0.f) {
                    continue;
                }
                nodes[i].massCenter.x += child.massCenter.x * child.mass;
                nodes[i].massCenter.y += child.massCenter.y * child.mass;
                nodes[i].mass += child.mass;
            }
            if (nodes[i].mass != 0.f) {
                nodes[i].massCenter.x /= nodes[i].mass;
                nodes[i].massCenter.y /= nodes[i].mass;
            }
        }
    }
}

bool ComputeBackend::contains(const QuadTreeNode *node,
                              const Particle *particle) {
    return particle->position.x >= node->position.x &&
           particle->position.y >= node->position.y &&
           particle->position.x <= node->position.x + node->dimensions.x &&
           particle->position.y <= node->position.y + node->dimensions.y;
}

int ComputeBackend::childIndex(const QuadTreeNode *node,
                               const Particle *particle) {
    if (!contains(node, particle)) {
        fatal("particle is not inside the node used for child selection");
    }
    const float midX = node->position.x + node->dimensions.x * 0.5f;
    const float midY = node->position.y + node->dimensions.y * 0.5f;
    int index = 0;
    if (particle->position.x >= midX) {
        index += 1;
    }
    if (particle->position.y >= midY) {
        index += 2;
    }
    return index;
}

std::unique_ptr<ComputeBackend>
createComputeBackend(ComputeBackendKind kind,
                     std::vector<Particle> &particles) {
    switch (kind) {
    case ComputeBackendKind::OpenCL:
        return std::make_unique<OpenClBackend>(particles);
    case ComputeBackendKind::OpenMP:
        return std::make_unique<OpenMpBackend>(particles);
    }
    fatal("unknown compute backend");
}
