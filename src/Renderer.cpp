#include "Renderer.h"
#include "Fail.h"

Renderer::Renderer(unsigned int windowWidth, unsigned int windowHeight) {
    _window = sf::RenderWindow(sf::VideoMode({windowWidth, windowHeight}),
                               "QuadTrees");
}

Renderer::~Renderer() = default;

void Renderer::pollClose() {
    while (const auto event = _window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            _window.close();
        }
    }
}

bool Renderer::isOpen() { return _window.isOpen(); }

void Renderer::draw(std::vector<Particle> &particles,
                    std::vector<QuadTreeNode> &nodes, bool transform,
                    bool showGrid, bool showMassCenter) {
    float2 scale;
    float2 translation;
    float particleScale;

    _window.clear(sf::Color::Black);
    if (transform) {
        if (nodes[0].dimensions.x <= 0.f || nodes[0].dimensions.y <= 0.f) {
            warn("skipped view transform because the root cell has "
                 "non-positive size");
            transform = false;
        }
    }
    if (transform) {
        scale = {_window.getSize().x / nodes[0].dimensions.x,
                 _window.getSize().y / nodes[0].dimensions.y};
        translation = {-nodes[0].position.x * scale.x,
                       -nodes[0].position.y * scale.y};
        particleScale = 1 / nodes[0].dimensions.x;
    } else {
        scale = {_window.getSize().x / 1.f, _window.getSize().y / 1.f};
        translation = {0.f * scale.x, 0.f * scale.y};
        particleScale = 1;
    }

    if (showGrid) {
        for (QuadTreeNode &node : nodes) {
            sf::RectangleShape r(sf::Vector2f(node.dimensions.x * scale.x,
                                              node.dimensions.y * scale.y));
            r.setPosition(
                sf::Vector2f(node.position.x * scale.x + translation.x,
                             node.position.y * scale.y + translation.y));
            r.setOutlineThickness(1.f);
            r.setFillColor(sf::Color::Black);
            r.setOutlineColor(sf::Color(50, 50, 50));
            _window.draw(r);
        }
    }

    for (Particle &particle : particles) {
        double radius = std::max(particle.mass * 5.f * particleScale, 1.f);
        sf::CircleShape c(radius);
        c.setPosition(sf::Vector2f(
            particle.position.x * scale.x - radius + translation.x,
            particle.position.y * scale.y - radius + translation.y));
        c.setFillColor(sf::Color::White);
        _window.draw(c);
    }

    if (showMassCenter) {
        int radius = 5;
        sf::CircleShape c(radius);
        c.setPosition(sf::Vector2f(
            nodes[0].massCenter.x * scale.x - radius + translation.x,
            nodes[0].massCenter.y * scale.y - radius + translation.y));
        c.setFillColor(sf::Color::Red);
        _window.draw(c);
    }
    _window.display();
}