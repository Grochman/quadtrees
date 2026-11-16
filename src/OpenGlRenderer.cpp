#include "OpenGlRenderer.h"
#include "Fail.h"

#include <SFML/Window/Context.hpp>
#include <algorithm>
#include <cstddef>
#include <string>

#define GL_GLEXT_PROTOTYPES 0
#include <GL/glcorearb.h>

namespace {
constexpr char vertSource[] = {
#embed "shaders/draw.vert"
    , '\0'};
constexpr char fragSource[] = {
#embed "shaders/draw.frag"
    , '\0'};

struct Gl {
    PFNGLVIEWPORTPROC Viewport;
    PFNGLCLEARCOLORPROC ClearColor;
    PFNGLCLEARPROC Clear;
    PFNGLGENVERTEXARRAYSPROC GenVertexArrays;
    PFNGLDELETEVERTEXARRAYSPROC DeleteVertexArrays;
    PFNGLBINDVERTEXARRAYPROC BindVertexArray;
    PFNGLGENBUFFERSPROC GenBuffers;
    PFNGLDELETEBUFFERSPROC DeleteBuffers;
    PFNGLBINDBUFFERPROC BindBuffer;
    PFNGLBUFFERDATAPROC BufferData;
    PFNGLENABLEVERTEXATTRIBARRAYPROC EnableVertexAttribArray;
    PFNGLVERTEXATTRIBPOINTERPROC VertexAttribPointer;
    PFNGLCREATESHADERPROC CreateShader;
    PFNGLSHADERSOURCEPROC ShaderSource;
    PFNGLCOMPILESHADERPROC CompileShader;
    PFNGLGETSHADERIVPROC GetShaderiv;
    PFNGLGETSHADERINFOLOGPROC GetShaderInfoLog;
    PFNGLDELETESHADERPROC DeleteShader;
    PFNGLCREATEPROGRAMPROC CreateProgram;
    PFNGLATTACHSHADERPROC AttachShader;
    PFNGLLINKPROGRAMPROC LinkProgram;
    PFNGLGETPROGRAMIVPROC GetProgramiv;
    PFNGLGETPROGRAMINFOLOGPROC GetProgramInfoLog;
    PFNGLDELETEPROGRAMPROC DeleteProgram;
    PFNGLUSEPROGRAMPROC UseProgram;
    PFNGLDRAWARRAYSPROC DrawArrays;
} gl;

template <typename T> T loadGlFn(const char *name) {
    const sf::GlFunctionPointer fn = sf::Context::getFunction(name);
    if (fn == nullptr) {
        fatal("missing OpenGL function");
    }
    return reinterpret_cast<T>(fn);
}

void pushVertex(std::vector<float> &verts, float x, float y, float r, float g,
                float b, float u, float v) {
    verts.push_back(x);
    verts.push_back(y);
    verts.push_back(r);
    verts.push_back(g);
    verts.push_back(b);
    verts.push_back(u);
    verts.push_back(v);
}

void pushQuad(std::vector<float> &verts, float x, float y, float rx, float ry,
              float r, float g, float b) {
    const float x0 = x - rx;
    const float x1 = x + rx;
    const float y0 = y - ry;
    const float y1 = y + ry;
    pushVertex(verts, x0, y0, r, g, b, -1.f, -1.f);
    pushVertex(verts, x1, y0, r, g, b, 1.f, -1.f);
    pushVertex(verts, x1, y1, r, g, b, 1.f, 1.f);
    pushVertex(verts, x0, y0, r, g, b, -1.f, -1.f);
    pushVertex(verts, x1, y1, r, g, b, 1.f, 1.f);
    pushVertex(verts, x0, y1, r, g, b, -1.f, 1.f);
}

constexpr int floatsPerVertex = 7;
} // namespace

void OpenGlRenderer::loadGl() {
    gl.Viewport = loadGlFn<PFNGLVIEWPORTPROC>("glViewport");
    gl.ClearColor = loadGlFn<PFNGLCLEARCOLORPROC>("glClearColor");
    gl.Clear = loadGlFn<PFNGLCLEARPROC>("glClear");
    gl.GenVertexArrays = loadGlFn<PFNGLGENVERTEXARRAYSPROC>("glGenVertexArrays");
    gl.DeleteVertexArrays =
        loadGlFn<PFNGLDELETEVERTEXARRAYSPROC>("glDeleteVertexArrays");
    gl.BindVertexArray = loadGlFn<PFNGLBINDVERTEXARRAYPROC>("glBindVertexArray");
    gl.GenBuffers = loadGlFn<PFNGLGENBUFFERSPROC>("glGenBuffers");
    gl.DeleteBuffers = loadGlFn<PFNGLDELETEBUFFERSPROC>("glDeleteBuffers");
    gl.BindBuffer = loadGlFn<PFNGLBINDBUFFERPROC>("glBindBuffer");
    gl.BufferData = loadGlFn<PFNGLBUFFERDATAPROC>("glBufferData");
    gl.EnableVertexAttribArray =
        loadGlFn<PFNGLENABLEVERTEXATTRIBARRAYPROC>("glEnableVertexAttribArray");
    gl.VertexAttribPointer =
        loadGlFn<PFNGLVERTEXATTRIBPOINTERPROC>("glVertexAttribPointer");
    gl.CreateShader = loadGlFn<PFNGLCREATESHADERPROC>("glCreateShader");
    gl.ShaderSource = loadGlFn<PFNGLSHADERSOURCEPROC>("glShaderSource");
    gl.CompileShader = loadGlFn<PFNGLCOMPILESHADERPROC>("glCompileShader");
    gl.GetShaderiv = loadGlFn<PFNGLGETSHADERIVPROC>("glGetShaderiv");
    gl.GetShaderInfoLog =
        loadGlFn<PFNGLGETSHADERINFOLOGPROC>("glGetShaderInfoLog");
    gl.DeleteShader = loadGlFn<PFNGLDELETESHADERPROC>("glDeleteShader");
    gl.CreateProgram = loadGlFn<PFNGLCREATEPROGRAMPROC>("glCreateProgram");
    gl.AttachShader = loadGlFn<PFNGLATTACHSHADERPROC>("glAttachShader");
    gl.LinkProgram = loadGlFn<PFNGLLINKPROGRAMPROC>("glLinkProgram");
    gl.GetProgramiv = loadGlFn<PFNGLGETPROGRAMIVPROC>("glGetProgramiv");
    gl.GetProgramInfoLog =
        loadGlFn<PFNGLGETPROGRAMINFOLOGPROC>("glGetProgramInfoLog");
    gl.DeleteProgram = loadGlFn<PFNGLDELETEPROGRAMPROC>("glDeleteProgram");
    gl.UseProgram = loadGlFn<PFNGLUSEPROGRAMPROC>("glUseProgram");
    gl.DrawArrays = loadGlFn<PFNGLDRAWARRAYSPROC>("glDrawArrays");
}

unsigned int OpenGlRenderer::compileProgram() {
    const auto compile = [](GLenum type, const char *src) {
        const GLuint shader = gl.CreateShader(type);
        gl.ShaderSource(shader, 1, &src, nullptr);
        gl.CompileShader(shader);
        GLint ok = 0;
        gl.GetShaderiv(shader, GL_COMPILE_STATUS, &ok);
        if (!ok) {
            GLint logSize = 0;
            gl.GetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize);
            std::string log(static_cast<size_t>(std::max(logSize, 1)), '\0');
            gl.GetShaderInfoLog(shader, logSize, nullptr, log.data());
            std::cerr << log << std::endl;
            fatal("OpenGL shader compile failed");
        }
        return shader;
    };

    const GLuint vs = compile(GL_VERTEX_SHADER, vertSource);
    const GLuint fs = compile(GL_FRAGMENT_SHADER, fragSource);
    const GLuint program = gl.CreateProgram();
    gl.AttachShader(program, vs);
    gl.AttachShader(program, fs);
    gl.LinkProgram(program);
    GLint ok = 0;
    gl.GetProgramiv(program, GL_LINK_STATUS, &ok);
    gl.DeleteShader(vs);
    gl.DeleteShader(fs);
    if (!ok) {
        GLint logSize = 0;
        gl.GetProgramiv(program, GL_INFO_LOG_LENGTH, &logSize);
        std::string log(static_cast<size_t>(std::max(logSize, 1)), '\0');
        gl.GetProgramInfoLog(program, logSize, nullptr, log.data());
        std::cerr << log << std::endl;
        fatal("OpenGL program link failed");
    }
    return program;
}

OpenGlRenderer::OpenGlRenderer(unsigned int windowWidth,
                               unsigned int windowHeight, bool transform,
                               bool showGrid, bool showMassCenter)
    : _transform(transform), _showGrid(showGrid),
      _showMassCenter(showMassCenter) {
    sf::ContextSettings settings;
    settings.majorVersion = 3;
    settings.minorVersion = 3;
    settings.attributeFlags = sf::ContextSettings::Core;
    _window = sf::Window(sf::VideoMode({windowWidth, windowHeight}),
                         "QuadTrees", sf::Style::Default, sf::State::Windowed,
                         settings);
    if (!_window.setActive(true)) {
        fatal("failed to activate OpenGL context");
    }

    loadGl();
    gl.Viewport(0, 0, static_cast<GLsizei>(windowWidth),
                static_cast<GLsizei>(windowHeight));
    gl.ClearColor(0.f, 0.f, 0.f, 1.f);

    _program = compileProgram();
    gl.GenVertexArrays(1, &_vao);
    gl.GenBuffers(1, &_vbo);
    gl.BindVertexArray(_vao);
    gl.BindBuffer(GL_ARRAY_BUFFER, _vbo);
    const GLsizei stride = floatsPerVertex * static_cast<GLsizei>(sizeof(float));
    gl.EnableVertexAttribArray(0);
    gl.VertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, nullptr);
    gl.EnableVertexAttribArray(1);
    gl.VertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride,
                           reinterpret_cast<void *>(2 * sizeof(float)));
    gl.EnableVertexAttribArray(2);
    gl.VertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride,
                           reinterpret_cast<void *>(5 * sizeof(float)));
}

OpenGlRenderer::~OpenGlRenderer() {
    if (_window.setActive(true)) {
        if (_vbo != 0) {
            gl.DeleteBuffers(1, &_vbo);
        }
        if (_vao != 0) {
            gl.DeleteVertexArrays(1, &_vao);
        }
        if (_program != 0) {
            gl.DeleteProgram(_program);
        }
    }
}

void OpenGlRenderer::pollClose() {
    while (const auto event = _window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            _window.close();
        } else if (const auto *resized = event->getIf<sf::Event::Resized>()) {
            if (_window.setActive(true)) {
                gl.Viewport(0, 0, static_cast<GLsizei>(resized->size.x),
                            static_cast<GLsizei>(resized->size.y));
            }
        }
    }
}

bool OpenGlRenderer::isOpen() { return _window.isOpen(); }

void OpenGlRenderer::setView(const std::vector<QuadTreeNode> &nodes) {
    bool transform = _transform;
    const auto size = _window.getSize();
    if (transform && nodes.empty()) {
        warn("skipped view transform because the tree is empty");
        transform = false;
    }
    if (transform) {
        if (nodes[0].dimensions.x <= 0.f || nodes[0].dimensions.y <= 0.f) {
            warn("skipped view transform because the root cell has "
                 "non-positive size");
            transform = false;
        }
    }
    if (transform) {
        _scale = {size.x / nodes[0].dimensions.x,
                  size.y / nodes[0].dimensions.y};
        _translation = {-nodes[0].position.x * _scale.x,
                        -nodes[0].position.y * _scale.y};
        _particleScale = 1.f / nodes[0].dimensions.x;
    } else {
        _scale = {static_cast<float>(size.x), static_cast<float>(size.y)};
        _translation = {0.f, 0.f};
        _particleScale = 1.f;
    }
}

void OpenGlRenderer::draw(std::vector<Particle> &particles,
                          std::vector<QuadTreeNode> &nodes) {
    if (!_window.isOpen()) {
        return;
    }
    if (!_window.setActive(true)) {
        fatal("failed to activate OpenGL context");
    }

    setView(nodes);
    const auto size = _window.getSize();
    const float width = static_cast<float>(size.x);
    const float height = static_cast<float>(size.y);

    const auto toNdc = [&](float worldX, float worldY) {
        const float px = worldX * _scale.x + _translation.x;
        const float py = worldY * _scale.y + _translation.y;
        return float2{2.f * px / width - 1.f, 1.f - 2.f * py / height};
    };

    _verts.clear();
    size_t lineCount = 0;
    if (_showGrid) {
        for (const QuadTreeNode &node : nodes) {
            const float2 p0 = toNdc(node.position.x, node.position.y);
            const float2 p1 =
                toNdc(node.position.x + node.dimensions.x, node.position.y);
            const float2 p2 = toNdc(node.position.x + node.dimensions.x,
                                    node.position.y + node.dimensions.y);
            const float2 p3 =
                toNdc(node.position.x, node.position.y + node.dimensions.y);
            const float r = 50.f / 255.f;
            pushVertex(_verts, p0.x, p0.y, r, r, r, 0.f, 0.f);
            pushVertex(_verts, p1.x, p1.y, r, r, r, 0.f, 0.f);
            pushVertex(_verts, p1.x, p1.y, r, r, r, 0.f, 0.f);
            pushVertex(_verts, p2.x, p2.y, r, r, r, 0.f, 0.f);
            pushVertex(_verts, p2.x, p2.y, r, r, r, 0.f, 0.f);
            pushVertex(_verts, p3.x, p3.y, r, r, r, 0.f, 0.f);
            pushVertex(_verts, p3.x, p3.y, r, r, r, 0.f, 0.f);
            pushVertex(_verts, p0.x, p0.y, r, r, r, 0.f, 0.f);
            lineCount += 8;
        }
    }

    const size_t quadBegin = lineCount;
    size_t quadCount = 0;
    for (const Particle &particle : particles) {
        const float2 p = toNdc(particle.position.x, particle.position.y);
        const float radius =
            std::max(particle.mass * 5.f * _particleScale, 1.f);
        pushQuad(_verts, p.x, p.y, 2.f * radius / width,
                 2.f * radius / height, 1.f, 1.f, 1.f);
        quadCount += 6;
    }
    if (_showMassCenter && !nodes.empty()) {
        const float2 p =
            toNdc(nodes[0].massCenter.x, nodes[0].massCenter.y);
        pushQuad(_verts, p.x, p.y, 10.f / width, 10.f / height, 1.f, 0.f,
                 0.f);
        quadCount += 6;
    }

    gl.Clear(GL_COLOR_BUFFER_BIT);
    gl.UseProgram(_program);
    gl.BindVertexArray(_vao);
    gl.BindBuffer(GL_ARRAY_BUFFER, _vbo);
    if (!_verts.empty()) {
        gl.BufferData(GL_ARRAY_BUFFER,
                      static_cast<GLsizeiptr>(_verts.size() * sizeof(float)),
                      _verts.data(), GL_STREAM_DRAW);
    }

    if (lineCount > 0) {
        gl.DrawArrays(GL_LINES, 0, static_cast<GLsizei>(lineCount));
    }
    if (quadCount > 0) {
        gl.DrawArrays(GL_TRIANGLES, static_cast<GLsizei>(quadBegin),
                      static_cast<GLsizei>(quadCount));
    }

    _window.display();
}
