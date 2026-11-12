#pragma once

#include "Particle.h"
#include "QuadTreeNode.h"
#include <SFML/Graphics.hpp>

class Renderer {
  public:
    Renderer(unsigned int windowWidth, unsigned int windowHeight,
             bool transform, bool showGrid, bool showMassCenter);
    ~Renderer();
    void pollClose();
    bool isOpen();
    void draw(std::vector<Particle> &particles,
              std::vector<QuadTreeNode> &nodes);

  private:
    sf::RenderWindow _window;
    bool _transform;
    bool _showGrid;
    bool _showMassCenter;
};
