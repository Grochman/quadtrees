#pragma once

#include "Particle.h"
#include "QuadTreeNode.h"
#include <SFML/Window.hpp>
#include <vector>

class OpenGlRenderer {
  public:
    OpenGlRenderer(unsigned int windowWidth, unsigned int windowHeight,
                   bool transform, bool showGrid, bool showMassCenter);
    ~OpenGlRenderer();
    OpenGlRenderer(const OpenGlRenderer &) = delete;
    OpenGlRenderer &operator=(const OpenGlRenderer &) = delete;
    OpenGlRenderer(OpenGlRenderer &&) = delete;
    OpenGlRenderer &operator=(OpenGlRenderer &&) = delete;

    void pollClose();
    bool isOpen();
    void draw(std::vector<Particle> &particles,
              std::vector<QuadTreeNode> &nodes);

  private:
    void loadGl();
    unsigned int compileProgram();
    void setView(const std::vector<QuadTreeNode> &nodes);

    sf::Window _window;
    bool _transform;
    bool _showGrid;
    bool _showMassCenter;
    unsigned int _vao = 0;
    unsigned int _vbo = 0;
    unsigned int _program = 0;
    float2 _scale = {1.f, 1.f};
    float2 _translation = {0.f, 0.f};
    float _particleScale = 1.f;
    std::vector<float> _verts;
};
