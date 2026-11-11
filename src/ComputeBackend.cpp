#include "ComputeBackend.h"
#include "Fail.h"

#include <cmath>
#include <iostream>
#include <string>

namespace {
constexpr char kUpdatePositions[] = {
#embed "kernels/updatePositions.cl"
    , '\0'};
constexpr char kComputeForces[] = {
#embed "kernels/computeForces.cl"
    , '\0'};
} // namespace

ComputeBackend::ComputeBackend(std::vector<Particle> &particles)
    : _particles(particles), _context(nullptr), _queue(nullptr),
      _particleBuffer(nullptr), _nodeBuffer(nullptr),
      _updatePositionsKernel(nullptr), _computeForcesKernel(nullptr) {

    cl_int status = CL_SUCCESS;
    cl_uint platformCount = 0;
    status = clGetPlatformIDs(0, nullptr, &platformCount);
    checkError(status, "clGetPlatformIDs count");
    if (platformCount == 0) {
        fatal("no OpenCL platform found");
    }

    cl_platform_id platform = nullptr;
    status = clGetPlatformIDs(1, &platform, nullptr);
    checkError(status, "clGetPlatformIDs");

    cl_device_id device = nullptr;
    status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, nullptr);
    checkError(status, "clGetDeviceIDs GPU");

    _context = clCreateContext(nullptr, 1, &device, nullptr, nullptr, &status);
    checkError(status, "clCreateContext");
    _queue =
        clCreateCommandQueueWithProperties(_context, device, nullptr, &status);
    checkError(status, "clCreateCommandQueueWithProperties");

    _particleBuffer =
        clCreateBuffer(_context, CL_MEM_READ_WRITE,
                       particles.size() * sizeof(Particle), nullptr, &status);
    checkError(status, "clCreateBuffer particles");
    _globalDimensions = particles.size();

    _updatePositionsKernel =
        buildKernel(device, kUpdatePositions, "updatePositions");
    _computeForcesKernel = buildKernel(device, kComputeForces, "computeForces");
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
    if (_queue != nullptr) {
        checkError(clReleaseCommandQueue(_queue), "clReleaseCommandQueue");
        _queue = nullptr;
    }
    if (_context != nullptr) {
        checkError(clReleaseContext(_context), "clReleaseContext");
        _context = nullptr;
    }
}

void ComputeBackend::computeForces(std::vector<QuadTreeNode> &nodes) {
    if (_nodeBuffer != nullptr) {
        checkError(clReleaseMemObject(_nodeBuffer), "clReleaseMemObject nodes");
        _nodeBuffer = nullptr;
    }
    cl_int status = CL_SUCCESS;
    _nodeBuffer =
        clCreateBuffer(_context, CL_MEM_READ_WRITE,
                       nodes.size() * sizeof(QuadTreeNode), nullptr, &status);
    checkError(status, "clCreateBuffer nodes");

    checkError(clEnqueueWriteBuffer(_queue, _particleBuffer, CL_TRUE, 0,
                                    _particles.size() * sizeof(Particle),
                                    _particles.data(), 0, nullptr, nullptr),
               "clEnqueueWriteBuffer particles");
    checkError(clEnqueueWriteBuffer(_queue, _nodeBuffer, CL_TRUE, 0,
                                    nodes.size() * sizeof(QuadTreeNode),
                                    nodes.data(), 0, nullptr, nullptr),
               "clEnqueueWriteBuffer nodes");

    checkError(clSetKernelArg(_computeForcesKernel, 0, sizeof(cl_mem),
                              &_particleBuffer),
               "clSetKernelArg forces particles");
    checkError(
        clSetKernelArg(_computeForcesKernel, 1, sizeof(cl_mem), &_nodeBuffer),
        "clSetKernelArg forces nodes");

    checkError(clEnqueueNDRangeKernel(_queue, _computeForcesKernel, 1, nullptr,
                                      &_globalDimensions, nullptr, 0, nullptr,
                                      nullptr),
               "clEnqueueNDRangeKernel computeForces");
    checkError(clFinish(_queue), "clFinish computeForces");
}

void ComputeBackend::updatePositions(float dt) {
    if (!std::isfinite(dt) || dt < 0.f) {
        fatal("non-finite or negative integration step");
    }
    if (dt == 0.f) {
        warn("skipped position update because dt is zero");
        return;
    }
    checkError(clSetKernelArg(_updatePositionsKernel, 0, sizeof(cl_mem),
                              &_particleBuffer),
               "clSetKernelArg update particles");
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
    checkError(clFinish(_queue), "clFinish updatePositions");
}

cl_kernel ComputeBackend::buildKernel(cl_device_id device, const char *source,
                                      const char *functionName) {

    cl_int status = CL_SUCCESS;
    cl_program program = clCreateProgramWithSource(
        _context, 1, (const char **)&source, nullptr, &status);
    checkError(status, "clCreateProgramWithSource");
    status = clBuildProgram(program, 1, &device, nullptr, nullptr, nullptr);
    checkBuildError(status, program, device);

    cl_kernel kernel = clCreateKernel(program, functionName, &status);
    checkError(status, "clCreateKernel");
    return kernel;
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
