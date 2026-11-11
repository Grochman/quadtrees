#include "Particle.h"
#include "QuadTree.h"
#include <SFML/Graphics.hpp>

class Renderer {
  public:
    Renderer(unsigned int windowWidth, unsigned int windowHeight);
    ~Renderer();
    void pollClose();
    bool isOpen();
    void draw(std::vector<Particle> &particles,
              std::vector<QuadTreeNode> &nodes, bool transform, bool showGrid,
              bool showMassCenter);

  private:
    sf::RenderWindow _window;
};