#pragma once
#include "QuadTreeNode.h"

template<unsigned int MaxCapacity>
class QuadTreeNodeImp : public QuadTreeNode {
    static constexpr unsigned int _maxCapacity = MaxCapacity;
    void split();
    void deleteLeaves();
    void merge();
    QuadTreeNodeImp(const Vector2d& position, const Vector2d& dimentions, QuadTreeNode* const parent) :
        QuadTreeNode(position, dimentions, parent) {};
public:
    QuadTreeNodeImp(const Vector2d& position, const Vector2d& dimentions) : QuadTreeNode(position, dimentions) {};
    QuadTreeNodeImp() : QuadTreeNode() {};
    ~QuadTreeNodeImp();
    void addParticle(Particle* const particle) override;
    void updateParticleOwnership(Particle* const particle) override;
    std::unordered_set<Particle*> query(const Vector2d& position, const double radius);
    unsigned int getMaxCapacity() { return _maxCapacity; };
};


#include "QuadTreeNodeImp.inl"