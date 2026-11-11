#include <cerrno>
#include <chrono>
#include <climits>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <iostream>

#include "Fail.h"
#include "Renderer.h"
#include "World.h"

namespace {
using Clock = std::chrono::steady_clock;

constexpr unsigned int defaultParticleCount = 256;
constexpr unsigned int windowSize = 600;
constexpr float timeScale = 0.01f;

struct Options {
    unsigned int particleCount = defaultParticleCount;
    bool transform = false;
    bool showGrid = true;
    bool showMassCenter = false;
};

unsigned int parseCount(const char *text) {
    char *end = nullptr;
    errno = 0;
    const long parsed = std::strtol(text, &end, 10);
    if (end == text || *end != '\0' || errno == ERANGE) {
        fatal("particle count must be a non-negative integer");
    }
    if (parsed < 0) {
        fatal("particle count must not be negative");
    }
    if (parsed > static_cast<long>(UINT_MAX)) {
        fatal("particle count is too large");
    }
    return static_cast<unsigned int>(parsed);
}

Options parseArgs(int argc, char **argv) {
    Options options;
    bool countSet = false;

    for (int i = 1; i < argc; i++) {
        if (std::strcmp(argv[i], "--transform") == 0) {
            options.transform = true;
        } else if (std::strcmp(argv[i], "--grid") == 0) {
            options.showGrid = true;
        } else if (std::strcmp(argv[i], "--no-grid") == 0) {
            options.showGrid = false;
        } else if (std::strcmp(argv[i], "--mass-center") == 0) {
            options.showMassCenter = true;
        } else if (argv[i][0] == '-') {
            fatal("usage: quadtrees [particleCount] [--transform] [--grid] "
                  "[--no-grid] [--mass-center]");
        } else if (countSet) {
            fatal("usage: quadtrees [particleCount] [--transform] [--grid] "
                  "[--no-grid] [--mass-center]");
        } else {
            options.particleCount = parseCount(argv[i]);
            countSet = true;
        }
    }

    return options;
}

float restart(Clock::time_point &mark) {
    const Clock::time_point now = Clock::now();
    const float seconds =
        std::chrono::duration_cast<std::chrono::microseconds>(now - mark)
            .count() /
        1000000.f;
    mark = now;
    return seconds;
}
} // namespace

int main(int argc, char **argv) {
    srand(time(NULL));

    const Options options = parseArgs(argc, argv);
    World world(options.particleCount);
    Renderer renderer(windowSize, windowSize, options.transform,
                      options.showGrid, options.showMassCenter);

    Clock::time_point lastStep = Clock::now();

    while (renderer.isOpen()) {
        renderer.pollClose();

        const float dt = restart(lastStep);
        world.step(dt * timeScale);

        if (dt > 0.f) {
            std::cout << "steps/s: " << 1.f / dt << '\n';
        }
        renderer.draw(world.getParticles(), world.getNodes());
    }

    return 0;
}
