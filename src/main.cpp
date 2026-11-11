#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iostream>

#include "Renderer.h"
#include "World.h"

namespace {
using Clock = std::chrono::steady_clock;

#ifdef NDEBUG
constexpr unsigned int particleCount = 1024;
#else
constexpr unsigned int particleCount = 256;
#endif

constexpr unsigned int windowSize = 600;
constexpr float timeScale = 0.01f;

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

int main() {
    srand(time(NULL));

    World world(particleCount);
    Renderer renderer(windowSize, windowSize);

    Clock::time_point lastStep = Clock::now();

    while (renderer.isOpen()) {
        renderer.pollClose();

        const float dt = restart(lastStep);
        world.step(dt * timeScale);

        if (dt > 0.f) {
            std::cout << "steps/s: " << 1.f / dt << '\n';
        }
        renderer.draw(world.getParticles(), world.getNodes(), false, true,
                      false);
    }

    return 0;
}
