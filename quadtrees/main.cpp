#include <iostream>
#include <chrono>
#include <cstdlib>
#include <string>
#include <algorithm>
#include <iomanip>
#include <cmath>
#include <ctime>
#include <omp.h>
#include <SFML/Graphics.hpp>
#include <SFML/Config.hpp>

#include "QuadTree.h"
#include "Particle.h"

namespace {

void printUsage(const char* argv0) {
    std::cout
        << "Usage: " << argv0 << " [options]\n"
        << "  --backend seq|omp"
#ifdef QUADTREES_ENABLE_OPENCL
        << "|opencl"
#endif
        << "   compute backend (default: omp)\n"
        << "  --particles N                 particle count (default: viz 1024/256, bench 8192)\n"
        << "  --steps N                     headless steps; omit to open the SFML window\n"
        << "  --threads N                   OpenMP thread count (default: OMP_NUM_THREADS / hardware)\n"
        << "  --dt F                        integration step (default: 0.01)\n"
        << "  --seed N                      RNG seed (default: 1 in bench, time in viz)\n"
        << "  --verify                      compare sequential vs OpenMP after --steps\n"
        << "  --help\n";
}

ComputeBackend parseBackend(const std::string& name) {
    if (name == "seq" || name == "sequential" || name == "cpu") {
        return ComputeBackend::Sequential;
    }
    if (name == "omp" || name == "openmp") {
        return ComputeBackend::OpenMP;
    }
#ifdef QUADTREES_ENABLE_OPENCL
    if (name == "opencl" || name == "gpu") {
        return ComputeBackend::OpenCL;
    }
#endif
    std::cerr << "Unknown backend: " << name << "\n";
    std::exit(1);
}

const char* backendName(ComputeBackend backend) {
    switch (backend) {
    case ComputeBackend::Sequential: return "sequential";
    case ComputeBackend::OpenMP: return "openmp";
#ifdef QUADTREES_ENABLE_OPENCL
    case ComputeBackend::OpenCL: return "opencl";
#endif
    }
    return "unknown";
}

std::vector<Particle> makeParticles(unsigned int count, unsigned int seed) {
    std::srand(seed);
    std::vector<Particle> particles;
    particles.reserve(count);
    for (unsigned int i = 0; i < count; i++) {
        particles.emplace_back();
        initParticle(&particles[i]);
    }
    return particles;
}

struct StepTimes {
    double bounds = 0;
    double build = 0;
    double mass = 0;
    double forces = 0;
    double update = 0;
    double total = 0;
};

StepTimes timeStep(QuadTree& tree, float dt) {
    StepTimes t;
    using clock = std::chrono::steady_clock;
    auto stamp = clock::now();
    auto take = [&](double& acc) {
        const auto now = clock::now();
        acc += std::chrono::duration<double>(now - stamp).count();
        stamp = now;
    };

    const auto start = stamp;
    tree.computeBounds();
    take(t.bounds);
    tree.build();
    take(t.build);
    tree.calculateMass();
    take(t.mass);
    tree.computeForces();
    take(t.forces);
    tree.updatePositions(dt);
    take(t.update);
    t.total = std::chrono::duration<double>(clock::now() - start).count();
    return t;
}

void add(StepTimes& a, const StepTimes& b) {
    a.bounds += b.bounds;
    a.build += b.build;
    a.mass += b.mass;
    a.forces += b.forces;
    a.update += b.update;
    a.total += b.total;
}

void printTimes(const char* label, const StepTimes& t, int steps) {
    const double inv = steps > 0 ? 1.0 / steps : 1.0;
    std::cout << std::fixed << std::setprecision(3)
        << label
        << "  total_ms=" << t.total * 1000.0
        << "  avg_ms=" << t.total * 1000.0 * inv
        << "  bounds=" << t.bounds * 1000.0 * inv
        << "  build=" << t.build * 1000.0 * inv
        << "  mass=" << t.mass * 1000.0 * inv
        << "  forces=" << t.forces * 1000.0 * inv
        << "  update=" << t.update * 1000.0 * inv
        << "\n";
}

int runBench(ComputeBackend backend, unsigned int particleCount, int steps, float dt, unsigned int seed) {
    auto particles = makeParticles(particleCount, seed);
    QuadTree tree(particles, backend);

    StepTimes warmup = timeStep(tree, dt);
    (void)warmup;

    StepTimes acc;
    for (int i = 0; i < steps; ++i) {
        add(acc, timeStep(tree, dt));
    }

    std::cout << "backend=" << backendName(backend)
        << " particles=" << particleCount
        << " steps=" << steps
        << " threads=" << omp_get_max_threads()
        << " dt=" << dt
        << " seed=" << seed
        << "\n";
    printTimes("timed", acc, steps);
    return 0;
}

int runVerify(unsigned int particleCount, int steps, float dt, unsigned int seed) {
    auto seqParticles = makeParticles(particleCount, seed);
    auto ompParticles = seqParticles;

    QuadTree seqTree(seqParticles, ComputeBackend::Sequential);
    QuadTree ompTree(ompParticles, ComputeBackend::OpenMP);

    for (int i = 0; i < steps; ++i) {
        seqTree.step(dt);
        ompTree.step(dt);
    }

    double maxPos = 0;
    double maxVel = 0;
    for (unsigned int i = 0; i < particleCount; ++i) {
        maxPos = std::max(maxPos, static_cast<double>(std::hypot(
            seqParticles[i].position.x - ompParticles[i].position.x,
            seqParticles[i].position.y - ompParticles[i].position.y)));
        maxVel = std::max(maxVel, static_cast<double>(std::hypot(
            seqParticles[i].velocity.x - ompParticles[i].velocity.x,
            seqParticles[i].velocity.y - ompParticles[i].velocity.y)));
    }

    std::cout << std::scientific
        << "verify particles=" << particleCount
        << " steps=" << steps
        << " max_pos_err=" << maxPos
        << " max_vel_err=" << maxVel
        << "\n";
    const double tol = 1e-4;
    if (maxPos > tol || maxVel > tol) {
        std::cerr << "OpenMP result diverged from sequential (tol=" << tol << ")\n";
        return 1;
    }
    std::cout << "ok\n";
    return 0;
}

void pumpEvents(sf::RenderWindow& window) {
#if SFML_VERSION_MAJOR >= 3
    while (const auto event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            window.close();
        }
    }
#else
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            window.close();
        }
    }
#endif
}

int runViz(ComputeBackend backend, unsigned int particleCount, float dt, unsigned int seed) {
    auto particles = makeParticles(particleCount, seed);
    QuadTree tree(particles, backend);

#if SFML_VERSION_MAJOR >= 3
    sf::RenderWindow window(sf::VideoMode({ 600, 600 }), "QuadTrees");
#else
    sf::RenderWindow window(sf::VideoMode(600, 600), "QuadTrees");
#endif

    std::cout << "viz backend=" << backendName(backend)
        << " particles=" << particleCount
        << " threads=" << omp_get_max_threads()
        << "\n";

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point beginFps = std::chrono::steady_clock::now();
    int frameCount = 0;

    while (window.isOpen()) {
        pumpEvents(window);
        frameCount++;
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        float timeElapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() / 1000000.f;
        begin = end;

        tree.step(timeElapsed * dt);

        std::chrono::steady_clock::time_point endFps = std::chrono::steady_clock::now();
        float frameTime = std::chrono::duration_cast<std::chrono::microseconds>(endFps - beginFps).count() / 1000000.f;

        if (frameCount / frameTime > 60) {
            beginFps = endFps;
            std::cout << "fps: " << frameCount / frameTime << '\n';

            window.clear(sf::Color::Black);
            tree.draw(window, false, true, false);
            window.display();
            frameCount = 0;
        }
    }

    return 0;
}

} // namespace

int main(int argc, char** argv) {
    ComputeBackend backend = ComputeBackend::OpenMP;
    bool haveParticles = false;
    unsigned int particleCount =
#ifdef NDEBUG
        1024;
#else
        256;
#endif
    int steps = -1;
    int threads = -1;
    float dt = 0.01f;
    bool haveSeed = false;
    unsigned int seed = 1;
    bool verify = false;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        auto need = [&](const char* name) -> const char* {
            if (i + 1 >= argc) {
                std::cerr << "Missing value for " << name << "\n";
                std::exit(1);
            }
            return argv[++i];
        };
        if (arg == "--help" || arg == "-h") {
            printUsage(argv[0]);
            return 0;
        }
        else if (arg == "--backend") {
            backend = parseBackend(need("--backend"));
        }
        else if (arg == "--particles") {
            particleCount = static_cast<unsigned int>(std::atoi(need("--particles")));
            haveParticles = true;
        }
        else if (arg == "--steps") {
            steps = std::atoi(need("--steps"));
        }
        else if (arg == "--threads") {
            threads = std::atoi(need("--threads"));
        }
        else if (arg == "--dt") {
            dt = std::strtof(need("--dt"), nullptr);
        }
        else if (arg == "--seed") {
            seed = static_cast<unsigned int>(std::atoi(need("--seed")));
            haveSeed = true;
        }
        else if (arg == "--verify") {
            verify = true;
        }
        else {
            std::cerr << "Unknown argument: " << arg << "\n";
            printUsage(argv[0]);
            return 1;
        }
    }

    if (threads > 0) {
        omp_set_num_threads(threads);
        omp_set_dynamic(0);
    }

    const bool headless = steps >= 0 || verify;
    if (headless && !haveParticles) {
        particleCount = 8192;
    }
    if (headless && steps < 0) {
        steps = 20;
    }
    if (!haveSeed && !headless) {
        seed = static_cast<unsigned int>(std::time(nullptr));
    }

    if (verify) {
        return runVerify(particleCount, steps, dt, seed);
    }
    if (headless) {
        return runBench(backend, particleCount, steps, dt, seed);
    }
    return runViz(backend, particleCount, dt, seed);
}
