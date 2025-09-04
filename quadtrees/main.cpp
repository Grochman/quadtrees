#include <iostream>
#include <chrono>
#include <CL/cl.h>
#include <SFML/Graphics.hpp>

#include "QuadTree.h"
#include "Particle.h"


int main()
{
    cl_platform_id platfmors;
    clGetPlatformIDs(1, &platfmors, nullptr);
    cl_device_id device;
    clGetDeviceIDs(platfmors, CL_DEVICE_TYPE_GPU, 1, &device, nullptr);

    cl_context context = clCreateContext(nullptr, 1, &device, nullptr, nullptr, nullptr);
    cl_command_queue queue = clCreateCommandQueueWithProperties(context, device, nullptr, nullptr);
    const char* source = {
        "typedef struct {\n"
        "    float2 position;\n"
        "    float2 velocity;\n"
        "    float2 acceleration;\n"
        "    float mass;\n"
        "    float _pad;\n"
        "} Particle;\n"
        "__kernel void updatePositions(__global Particle * particles, float dt) {\n"
        "   int id = get_global_id(0);\n"
        "   particles[id].velocity.x += particles[id].acceleration.x * dt * 0.5f;\n"
        "   particles[id].velocity.y += particles[id].acceleration.y * dt * 0.5f;\n"
        "   particles[id].position.x += particles[id].velocity.x * dt;\n"
        "   particles[id].position.y += particles[id].velocity.y * dt;\n"
        "   particles[id].velocity.x += particles[id].acceleration.x * dt * 0.5f;\n"
        "   particles[id].velocity.y += particles[id].acceleration.y * dt * 0.5f;\n"
        "   particles[id].acceleration.x = 0.0f;\n"
        "   particles[id].acceleration.y = 0.0f;\n"
        "}\n"
    };

    cl_program program = clCreateProgramWithSource(context, 1, (const char**)&source, nullptr, nullptr);
    cl_int status = clBuildProgram(program, 0, nullptr, nullptr, nullptr, nullptr);
    cl_kernel kernel = clCreateKernel(program, "updatePositions", nullptr);

    srand(time(NULL));
    const unsigned int particleCount = 100;

    std::vector<Particle> particles;
    particles.reserve(particleCount);
    for (int i = 0; i < particleCount; i++) {
        particles.emplace_back();
        initParticle(&particles[i]);
    }

    cl_mem buffer = clCreateBuffer(context, CL_MEM_READ_WRITE, particleCount * sizeof(Particle), nullptr, nullptr);
    size_t global_dimensions[] = { particleCount };

    QuadTree tree(particles);

    const sf::Vector2u windowDimentions = { 600,600 };
    sf::RenderWindow window(sf::VideoMode(windowDimentions), "QuadTrees");

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    
    while (window.isOpen()) {
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        float timeElapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() / 1000000.f;
        std::cout << "fps: " << 1 / timeElapsed << '\n';
        begin = end;
        
        tree.computeBounds(); // reduction
        tree.build(); // dick knows
        tree.calculateMass(); // reduction?
        // Apprioximately sort the bodies by spatial distnce missing
        tree.computeForces(); // simple paraler
        
        timeElapsed *= 0.01f;

        clEnqueueWriteBuffer(queue, buffer, CL_TRUE, 0, particleCount * sizeof(Particle), particles.data(), 0, nullptr, nullptr);
        clSetKernelArg(kernel, 0, sizeof(cl_mem), &buffer);
        clSetKernelArg(kernel, 1, sizeof(float), &timeElapsed);
        clEnqueueNDRangeKernel(queue, kernel, 1, nullptr, global_dimensions, nullptr, 0, nullptr, nullptr);
        clEnqueueReadBuffer(queue, buffer, CL_TRUE, 0, particleCount * sizeof(Particle), particles.data(), 0, nullptr, nullptr);
        clFinish(queue);
        //tree.updatePositions(timeElapsed * 0.01f); // simple parallel
        
        window.clear(sf::Color::Black);
        tree.draw(window, false, true, false);
        window.display();
    }

    return 0;
}
