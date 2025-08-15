#include <iostream>
#include <math.h>
#include <chrono>
#include <algorithm>

#include <SFML/Graphics.hpp>

#include "QuadTreeNode.h"
#include "Particle.h"


class QuadTree {
    std::vector<QuadTreeNode> nodes;

public:
    QuadTree() {};
    
    void reset() {
        nodes.clear();
    }
    
    void insert(std::vector<Particle>& particles) {
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
                    switch(i){
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
    
    void calculateMass() {

    }

    Vector2d calculateForce(const Particle& particle) {
        return { 0,0 };
    }
};



int main()
{
    srand(time(NULL));
    const unsigned int particleCount = 4;

    std::vector<Particle> particles;
    particles.reserve(particleCount);
    for (int i = 0; i < particleCount; i++) {
        particles.emplace_back();
        initParticle(&particles[i]);
    }
    
    QuadTree tree;

    tree.reset();
    tree.insert(particles);
    
    return 0;
}
