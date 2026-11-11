#include <SFML/Graphics.hpp>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iostream>

#include "Renderer.h"
#include "World.h"

int main() {
    srand(time(NULL));

#ifdef NDEBUG
    const unsigned int particleCount = 1024;
#else
    const unsigned int particleCount = 256;
#endif

    World world(particleCount);
    Renderer renderer(600, 600);

    std::chrono::steady_clock::time_point begin =
        std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point beginFps =
        std::chrono::steady_clock::now();

    int frameCount = 0;

    while (renderer.isOpen()) {
        renderer.pollClose();

        frameCount++;
        std::chrono::steady_clock::time_point end =
            std::chrono::steady_clock::now();
        float timeElapsed =
            std::chrono::duration_cast<std::chrono::microseconds>(end - begin)
                .count() /
            1000000.f;
        begin = end;

        world.step(timeElapsed * 0.01f);

        std::chrono::steady_clock::time_point endFps =
            std::chrono::steady_clock::now();
        float frameTime = std::chrono::duration_cast<std::chrono::microseconds>(
                              endFps - beginFps)
                              .count() /
                          1000000.f;
        if (frameTime <= 0.f) {
            continue;
        }

        if (frameCount / frameTime > 60) {
            beginFps = endFps;
            std::cout << "fps: " << frameCount / frameTime << '\n';
            renderer.draw(world.getParticles(), world.getNodes(), false, true,
                          false);
            frameCount = 0;
        }
    }

    return 0;
}
