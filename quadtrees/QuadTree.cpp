#include "QuadTree.h"

#include <stack>


QuadTree::QuadTree() {};

Vector2d QuadTree::calculateForce(Vector2d& distance, const double mass, Particle* particle) {
    Vector2d unit_vector = unit(distance);
    double distance_length = length(distance);
    distance_length = std::max(distance_length, 0.1);
    return { (mass * particle->mass / pow(distance_length, 2)) * unit_vector.x, (mass * particle->mass / pow(distance_length, 2)) * unit_vector.y };
}

void QuadTree::reset() {
    nodes.clear();
}

void QuadTree::computeBounds() {

}

void QuadTree::insert(std::vector<Particle>& particles) {
    // calculate and insert root node
    nodes.push_back(QuadTreeNode());
    // while particles insert
    for (Particle& particle : particles) {
        // go down the tree
        int idx = 0;
        while (nodes[idx].leavesIdx != -1) {
            int newIdx;
            for (int i = 0; i < 4; i++) {
                unsigned int leafIdx = nodes[idx].leavesIdx + i;
                if (contains(&nodes[leafIdx], &particle)) {
                    newIdx = leafIdx;
                }
            }
            idx = newIdx;
        }
        // split if needed
        while (nodes[idx].particle) {
            nodes[idx].leavesIdx = nodes.size();
            Particle* p = nodes[idx].particle;
            nodes[idx].particle = nullptr;
            for (int i = 0; i < 4; i++) {
                nodes.push_back(QuadTreeNode());
                int newIdx = nodes.size() - 1;
                nodes[newIdx].width = nodes[idx].width / 2;
                switch (i) {
                case 0:
                    nodes[newIdx].position = nodes[idx].position;
                    break;
                case 1:
                    nodes[newIdx].position = { nodes[idx].position.x + nodes[newIdx].width, nodes[idx].position.y };
                    break;
                case 2:
                    nodes[newIdx].position = { nodes[idx].position.x, nodes[idx].position.y + nodes[newIdx].width };
                    break;
                default:
                    nodes[newIdx].position = { nodes[idx].position.x + nodes[newIdx].width, nodes[idx].position.y + nodes[newIdx].width };
                }
                if (contains(&nodes[newIdx], p)) {
                    nodes[newIdx].particle = p;
                }
            }
            // go to correct quad
            for (int i = 0; i < 4; i++) {
                unsigned int leafIdx = nodes[idx].leavesIdx + i;
                if (contains(&nodes[leafIdx], &particle)) {
                    idx = leafIdx;
                }
            }
        }
        // insert
        nodes[idx].particle = &particle;
    }
}

void QuadTree::calculateMass() {
    for (int i = nodes.size() - 1; i >= 0; i--) {
        if (nodes[i].particle) {
            nodes[i].mass = nodes[i].particle->mass;
            nodes[i].massCenter = nodes[i].particle->position;
        }
        else if (nodes[i].leavesIdx == -1) {
            nodes[i].mass = 0;
            nodes[i].massCenter = { 0,0 };
        }
        else {
            nodes[i].massCenter = nodes[nodes[i].leavesIdx].massCenter;
            nodes[i].mass = nodes[nodes[i].leavesIdx].mass;
            for (int j = 1; j <= 3; j++) {
                nodes[i].massCenter = { nodes[i].massCenter.x * nodes[i].mass +
                    nodes[nodes[i].leavesIdx + j].massCenter.x * nodes[nodes[i].leavesIdx + j].mass,
                    nodes[i].massCenter.y * nodes[i].mass +
                    nodes[nodes[i].leavesIdx + j].massCenter.y * nodes[nodes[i].leavesIdx + j].mass };
                nodes[i].mass += nodes[nodes[i].leavesIdx].mass;
                nodes[i].massCenter.x /= nodes[i].mass;
                nodes[i].massCenter.y /= nodes[i].mass;
            }
        }
    }
}

Vector2d QuadTree::getTotalForce(Particle& particle) {
    Vector2d totalForce = { 0,0 };

    std::stack<int> idxs;
    idxs.push(0);

    while (!idxs.empty()) {
        int i = idxs.top();
        idxs.pop();

        if (nodes[i].particle == &particle) {
            continue;
        }
        if (nodes[i].particle) {
            Vector2d r = distance(particle.position, nodes[i].particle->position);
            totalForce = sum(totalForce, calculateForce(r, nodes[i].particle->mass, &particle));
            continue;
        }
        const double s = nodes[i].width;
        const double d = length(distance(particle.position, nodes[i].massCenter));
        if (s / d < 0.5) {
            Vector2d r = distance(particle.position, nodes[i].massCenter);
            totalForce = sum(totalForce, calculateForce(r, nodes[i].mass, &particle));
            continue;
        }
        else if (nodes[i].leavesIdx != -1) {
            idxs.push(nodes[i].leavesIdx);
            idxs.push(nodes[i].leavesIdx + 1);
            idxs.push(nodes[i].leavesIdx + 2);
            idxs.push(nodes[i].leavesIdx + 3);
        }
    }

    return totalForce;
}
