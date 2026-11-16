#include <algorithm>
#include <cerrno>
#include <chrono>
#include <climits>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>

#include "Fail.h"
#include "OpenGlRenderer.h"
#include "World.h"

namespace {
using Clock = std::chrono::steady_clock;

constexpr unsigned int defaultParticleCount = 256;
constexpr unsigned int windowSize = 600;
constexpr float stepsReportInterval = 1.f;
constexpr float tick = 0.01f;
constexpr float maxCatchUp = 0.1f;

constexpr const char *usage =
    "usage: quadtrees [particleCount] [--opencl|--openmp] [--no-transform] "
    "[--no-grid] [--mass-center] [--time-scale <s>]";

struct Options {
    unsigned int particleCount = defaultParticleCount;
    ComputeBackendKind backendKind = ComputeBackendKind::OpenCL;
    bool transform = true;
    bool showGrid = true;
    bool showMassCenter = false;
    float timeScale = 1;
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

float parseTimeScale(const char *text) {
    char *end = nullptr;
    errno = 0;
    const float parsed = std::strtof(text, &end);
    if (end == text || *end != '\0' || errno == ERANGE ||
        !std::isfinite(parsed) || parsed < 0.f) {
        fatal("time scale must be a non-negative finite number");
    }
    return parsed;
}

Options parseArgs(int argc, char **argv) {
    Options options;

    for (int i = 1; i < argc; i++) {
        if (std::strcmp(argv[i], "--opencl") == 0 ||
            std::strcmp(argv[i], "--openmp") == 0) {
            options.backendKind = std::strcmp(argv[i], "--openmp") == 0
                                      ? ComputeBackendKind::OpenMP
                                      : ComputeBackendKind::OpenCL;
        } else if (std::strcmp(argv[i], "--no-transform") == 0) {
            options.transform = false;
        } else if (std::strcmp(argv[i], "--no-grid") == 0) {
            options.showGrid = false;
        } else if (std::strcmp(argv[i], "--mass-center") == 0) {
            options.showMassCenter = true;
        } else if (std::strcmp(argv[i], "--time-scale") == 0) {
            if (i + 1 >= argc) {
                fatal(usage);
            }
            ++i;
            options.timeScale = parseTimeScale(argv[i]);
        } else if (argv[i][0] == '-') {
            fatal(usage);
        } else {
            options.particleCount = parseCount(argv[i]);
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
    OpenGlRenderer renderer(windowSize, windowSize, options.transform,
                            options.showGrid, options.showMassCenter);

    Clock::time_point lastStep = Clock::now();
    Clock::time_point lastReport = lastStep;
    int stepsSinceReport = 0;
    float lag = 0.f;

    while (renderer.isOpen()) {
        renderer.pollClose();

        lag += restart(lastStep);
        lag = std::min(lag, maxCatchUp);
        while (lag >= tick) {
            world.step(tick * options.timeScale);
            lag -= tick;
            ++stepsSinceReport;
        }

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
