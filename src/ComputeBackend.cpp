#include "ComputeBackend.h"
#include "Fail.h"

#include <cmath>
#include <iostream>
#include <string>
#include <vector>

namespace {
constexpr char typesSource[] = {
#embed "kernels/types.cl"
    , '\0'};
constexpr char updatePositionsSource[] = {
#embed "kernels/updatePositions.cl"
    , '\0'};
constexpr char computeForcesSource[] = {
#embed "kernels/computeForces.cl"
    , '\0'};
} // namespace

ComputeBackend::ComputeBackend(std::vector<Particle> &particles)
    : _particles(particles), _context(nullptr), _queue(nullptr),
      _program(nullptr), _particleBuffer(nullptr), _nodeBuffer(nullptr),
      _globalDimensions(particles.size()), _nodeCapacity(0),
      _updatePositionsKernel(nullptr), _computeForcesKernel(nullptr) {

    cl_int status = CL_SUCCESS;
    cl_device_id device = findGpu();

    _context = clCreateContext(nullptr, 1, &device, nullptr, nullptr, &status);
    checkError(status, "clCreateContext");
    _queue =
        clCreateCommandQueueWithProperties(_context, device, nullptr, &status);
    checkError(status, "clCreateCommandQueueWithProperties");

    if (_globalDimensions > 0) {
        _particleBuffer = clCreateBuffer(_context, CL_MEM_READ_WRITE,
                                         _globalDimensions * sizeof(Particle),
                                         nullptr, &status);
        checkError(status, "clCreateBuffer particles");
    }

    const char *sources[] = {typesSource, computeForcesSource,
                             updatePositionsSource};
    _program =
        clCreateProgramWithSource(_context, 3, sources, nullptr, &status);
    checkError(status, "clCreateProgramWithSource");
    status = clBuildProgram(_program, 1, &device, nullptr, nullptr, nullptr);
    checkBuildError(status, _program, device);

    _computeForcesKernel = clCreateKernel(_program, "computeForces", &status);
    checkError(status, "clCreateKernel computeForces");
    _updatePositionsKernel =
        clCreateKernel(_program, "updatePositions", &status);
    checkError(status, "clCreateKernel updatePositions");

    if (_particleBuffer != nullptr) {
        checkError(clSetKernelArg(_computeForcesKernel, 0, sizeof(cl_mem),
                                  &_particleBuffer),
                   "clSetKernelArg forces particles");
        checkError(clSetKernelArg(_updatePositionsKernel, 0, sizeof(cl_mem),
                                  &_particleBuffer),
                   "clSetKernelArg update particles");
    }
}

ComputeBackend::~ComputeBackend() {
    if (_nodeBuffer != nullptr) {
        checkError(clReleaseMemObject(_nodeBuffer), "clReleaseMemObject nodes");
        _nodeBuffer = nullptr;
    }
    if (_particleBuffer != nullptr) {
        checkError(clReleaseMemObject(_particleBuffer),
                   "clReleaseMemObject particles");
        _particleBuffer = nullptr;
    }
    if (_computeForcesKernel != nullptr) {
        checkError(clReleaseKernel(_computeForcesKernel),
                   "clReleaseKernel computeForces");
        _computeForcesKernel = nullptr;
    }
    if (_updatePositionsKernel != nullptr) {
        checkError(clReleaseKernel(_updatePositionsKernel),
                   "clReleaseKernel updatePositions");
        _updatePositionsKernel = nullptr;
    }
    if (_program != nullptr) {
        checkError(clReleaseProgram(_program), "clReleaseProgram");
        _program = nullptr;
    }
    if (_queue != nullptr) {
        checkError(clReleaseCommandQueue(_queue), "clReleaseCommandQueue");
        _queue = nullptr;
    }
    if (_context != nullptr) {
        checkError(clReleaseContext(_context), "clReleaseContext");
        _context = nullptr;
    }
}

cl_device_id ComputeBackend::findGpu() {
    cl_int status = CL_SUCCESS;
    cl_uint platformCount = 0;
    status = clGetPlatformIDs(0, nullptr, &platformCount);
    checkError(status, "clGetPlatformIDs count");
    if (platformCount == 0) {
        fatal("no OpenCL platform found");
    }

    std::vector<cl_platform_id> platforms(platformCount);
    status = clGetPlatformIDs(platformCount, platforms.data(), nullptr);
    checkError(status, "clGetPlatformIDs");

    for (cl_platform_id platform : platforms) {
        cl_device_id device = nullptr;
        status =
            clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, nullptr);
        if (status == CL_SUCCESS) {
            return device;
        }
    }

    fatal("no OpenCL GPU device found");
}

void ComputeBackend::ensureNodeBuffer(size_t nodeCount) {
    if (nodeCount <= _nodeCapacity) {
        return;
    }

    if (_nodeBuffer != nullptr) {
        checkError(clReleaseMemObject(_nodeBuffer), "clReleaseMemObject nodes");
        _nodeBuffer = nullptr;
    }
    cl_int status = CL_SUCCESS;
    _nodeBuffer =
        clCreateBuffer(_context, CL_MEM_READ_WRITE,
                       nodeCount * sizeof(QuadTreeNode), nullptr, &status);
    checkError(status, "clCreateBuffer nodes");
    _nodeCapacity = nodeCount;

    checkError(
        clSetKernelArg(_computeForcesKernel, 1, sizeof(cl_mem), &_nodeBuffer),
        "clSetKernelArg forces nodes");
}

void ComputeBackend::computeForces(std::vector<QuadTreeNode> &nodes) {
    if (_globalDimensions == 0) {
        return;
    }

    ensureNodeBuffer(nodes.size());

    checkError(clEnqueueWriteBuffer(_queue, _particleBuffer, CL_FALSE, 0,
                                    _particles.size() * sizeof(Particle),
                                    _particles.data(), 0, nullptr, nullptr),
               "clEnqueueWriteBuffer particles");
    checkError(clEnqueueWriteBuffer(_queue, _nodeBuffer, CL_FALSE, 0,
                                    nodes.size() * sizeof(QuadTreeNode),
                                    nodes.data(), 0, nullptr, nullptr),
               "clEnqueueWriteBuffer nodes");

    checkError(clEnqueueNDRangeKernel(_queue, _computeForcesKernel, 1, nullptr,
                                      &_globalDimensions, nullptr, 0, nullptr,
                                      nullptr),
               "clEnqueueNDRangeKernel computeForces");
}

void ComputeBackend::updatePositions(float dt) {
    if (!std::isfinite(dt) || dt < 0.f) {
        fatal("non-finite or negative integration step");
    }
    if (_globalDimensions == 0) {
        return;
    }
    if (dt == 0.f) {
        warn("skipped position update because dt is zero");
        checkError(clFinish(_queue), "clFinish skipped update");
        return;
    }

    checkError(clSetKernelArg(_updatePositionsKernel, 1, sizeof(float), &dt),
               "clSetKernelArg update dt");
    checkError(clEnqueueNDRangeKernel(_queue, _updatePositionsKernel, 1,
                                      nullptr, &_globalDimensions, nullptr, 0,
                                      nullptr, nullptr),
               "clEnqueueNDRangeKernel updatePositions");
    checkError(clEnqueueReadBuffer(_queue, _particleBuffer, CL_TRUE, 0,
                                   _particles.size() * sizeof(Particle),
                                   _particles.data(), 0, nullptr, nullptr),
               "clEnqueueReadBuffer particles");
}

void ComputeBackend::checkError(cl_int status, const char *what) {
    if (status != CL_SUCCESS) {
        std::cerr << "OpenCL error " << status << " in " << what << std::endl;
        abort();
    }
}

void ComputeBackend::checkBuildError(cl_int status, cl_program program,
                                     cl_device_id device) {
    if (status != CL_SUCCESS) {
        size_t logSize = 0;
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, nullptr,
                              &logSize);
        std::string log(logSize, '\0');
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, logSize,
                              log.data(), nullptr);
        std::cerr << log << std::endl;
        abort();
    }
}
