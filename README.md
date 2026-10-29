# quadtrees
N-body simulation done as a OpenCL learning exercise (all my notes: https://grochman.github.io/OpenCL_notes/). 

## features
- Barnes-Hut algorithm implementation
- Leapfrog integration
- quadtree visualization (SFML)
- OpenMP CPU multithreading (forces, bounding box, leapfrog)
- optional OpenCL GPU acceleration

## OpenMP

The default compute backend is OpenMP. Tree rebuild and mass/COM stay sequential; the Barnes-Hut force walk and leapfrog update run with OpenMP. Sequential CPU and OpenCL remain selectable for comparison.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
cd build
./quadtrees --help
./quadtrees --backend omp --particles 4096 --steps 30 --threads 4
./quadtrees --verify --particles 2048 --steps 10 --threads 4
./quadtrees --backend omp          # SFML window
```

Headless timing starts when `--steps` or `--verify` is set. Performance numbers and parallelisation notes: [docs/openmp-report.md](docs/openmp-report.md).

## Build

The simulation is C++20. It links SFML (3 preferred, 2.6 accepted) and OpenMP. OpenCL is optional: if the SDK is missing, CMake still builds the sequential and OpenMP backends. Run the binary from the `build` directory. The kernels are loaded as `kernels/*.cl` relative to the current directory, and the build copies them next to the executable. Use `CMAKE_BUILD_TYPE=Release` for the OpenMP measurements.

### Windows

Open `quadtrees.sln` in Visual Studio 2022 and build the x64 configuration. SFML comes from the vcpkg manifest in `vcpkg.json` (manifest mode). OpenCL comes from Intel oneAPI. The project file looks for headers and `OpenCL.lib` under `C:\Program Files (x86)\Intel\oneAPI\compiler\2025.2`.

### Linux with sudo

The default OpenCL package is `intel-opencl-icd`. It supports Intel Xe and Gen12 GPUs and newer.

```bash
sudo apt install build-essential cmake ninja-build pkg-config g++ \
  libsfml-dev opencl-headers ocl-icd-opencl-dev intel-opencl-icd clinfo
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=g++
cmake --build build
cd build && ./quadtrees --backend omp --particles 4096 --steps 20 --threads 4
```

For an older Intel iGPU (Gen8 through Gen11), install `intel-opencl-icd-legacy` instead of `intel-opencl-icd`. Do not install both. The program uses the first OpenCL platform it finds, so the extra runtime would hide the GPU.

### Linux without sudo

`setup-linux.sh` downloads the same packages with `apt-get download` and unpacks them into `.toolchain` in this directory. That directory is gitignored. `env-linux.sh` puts it on `PATH` and points the OpenCL loader at it. The default runtime is `intel-opencl-icd`. Pass `--legacy` to unpack `intel-opencl-icd-legacy` instead, for the same older Intel GPUs as above.

```bash
bash setup-linux.sh            # intel-opencl-icd
bash setup-linux.sh --legacy   # intel-opencl-icd-legacy
source env-linux.sh
cmake -S . -B build -G Ninja
cmake --build build
cd build && ./quadtrees
```

Source `env-linux.sh` again in every new shell before building or running.

## preview
<p align="center">
<img src="https://github.com/user-attachments/assets/1138bcb9-2cab-432b-afb0-1a53b9b5f116" width=300>

<img src="https://github.com/user-attachments/assets/f4974be8-922e-4953-bed3-422cc8ed4134" width=300>

<img src="https://github.com/user-attachments/assets/29dbb7b3-954c-41f4-94b6-67001e117b2b" width=300>
</p>

## branches
### main branches
- master - primary branch with wip gpu integration
- cpu - a more classic data structure approach
- OpenMP CPU backend lives on this tree (`--backend omp|seq`)
### outdated branches
- experimental - a quadtree data structure with the ability to update particle ownership without rebuilding the whole tree
- overengeneered - exploration of template usage for mor flexible quadtree object
- multithreading - naive, unfinished multithreading on cpu

## todo
- OpenCL acceleration for tree building and computation of the bounding box and mass center
- Implementation of particle sorting and further optimisation
- visualization improvements:
  - OpenGL
  - particle trails
  - density based
  - intuitive zoomout 
- exploration of different integration methods (rk4)
- collisions

## resources
- [The Barnes-Hut Algorithm](https://arborjs.org/docs/barnes-hut)
- [An Efficient CUDA Implementation of the Tree-Based Barnes Hut n-Body Algorithm](https://cs.wmich.edu/gupta/teaching/cs5260/5260Sp15web/studentProjects/white+loftus%20nBodySimulations/nbody-problem.pdf)
- [Leapfrog method and other “symplectic” algorithms for integrating Newton’s laws of motion](https://courses.physics.ucsd.edu/2019/Winter/physics141/Assignments/leapfrog.pdf)
