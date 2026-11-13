#include <cerrno>
#include <chrono>
#include <climits>
#include <cstdlib>
#include <cstring>
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
constexpr float stepsReportInterval = 1.f;

constexpr const char *usage =
    "usage: quadtrees [particleCount] [--opencl|--openmp] [--transform] "
    "[--grid] [--no-grid] [--mass-center]";

struct Options {
    unsigned int particleCount = defaultParticleCount;
    ComputeBackendKind backendKind = ComputeBackendKind::OpenCL;
    bool transform = false;
    bool showGrid = true;
    bool showMassCenter = false;
    bool backendSet = false;
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
        if (std::strcmp(argv[i], "--opencl") == 0 ||
            std::strcmp(argv[i], "--openmp") == 0) {
            if (options.backendSet) {
                fatal("specify only one of --opencl or --openmp");
            }
            options.backendKind = std::strcmp(argv[i], "--openmp") == 0
                                      ? ComputeBackendKind::OpenMP
                                      : ComputeBackendKind::OpenCL;
            options.backendSet = true;
        } else if (std::strcmp(argv[i], "--transform") == 0) {
            options.transform = true;
        } else if (std::strcmp(argv[i], "--grid") == 0) {
            options.showGrid = true;
        } else if (std::strcmp(argv[i], "--no-grid") == 0) {
            options.showGrid = false;
        } else if (std::strcmp(argv[i], "--mass-center") == 0) {
            options.showMassCenter = true;
        } else if (argv[i][0] == '-') {
            fatal(usage);
        } else if (countSet) {
            fatal(usage);
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

float timeSince(Clock::time_point &mark) {
    const Clock::time_point now = Clock::now();
    const float seconds =
        std::chrono::duration_cast<std::chrono::microseconds>(now - mark)
            .count() /
        1000000.f;
    return seconds;
}

} // namespace

int main(int argc, char **argv) {
    srand(time(NULL));

    const Options options = parseArgs(argc, argv);
    World world(options.particleCount, options.backendKind);
    Renderer renderer(windowSize, windowSize, options.transform,
                      options.showGrid, options.showMassCenter);

    Clock::time_point lastStep = Clock::now();
    Clock::time_point lastReport = lastStep;
    int stepsSinceReport = 0;

    while (renderer.isOpen()) {
        renderer.pollClose();

        const float dt = restart(lastStep);
        world.step(dt * timeScale);
        ++stepsSinceReport;

        const float reportElapsed = timeSince(lastReport);
        if (reportElapsed >= stepsReportInterval) {
            std::cout << "steps/s: " << stepsSinceReport / reportElapsed
                      << '\n';
            lastReport = Clock::now();
            stepsSinceReport = 0;
        }

        renderer.draw(world.getParticles(), world.getNodes());
    }

    return 0;
}
