#include "QuadTree.h"
#include "Fail.h"
#include <cmath>

#define ROUNDING_ERROR_CORRECTION 0.0001f
#define MIN_CELL_SIZE 1e-8f

QuadTree::QuadTree(std::vector<Particle> &particles) : _particles(particles) {}

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
