# OpenMP implementation report

**Topic:** Barnes–Hut N-body simulation (2D quadtree)  
**Environment:** multi-core CPU + OpenMP  
**Algorithm:** Barnes–Hut force evaluation + leapfrog integration (same as the existing CPU/OpenCL code)

## 1. Problem and sequential baseline

Each simulation step:

1. Compute the axis-aligned bounding box of all particles.
2. Rebuild the quadtree (one particle per leaf).
3. Accumulate mass and centre of mass from the leaves up.
4. Evaluate gravitational force on every particle with a Barnes–Hut tree walk (`θ = 0.5`, softening `0.1`).
5. Advance positions and velocities with leapfrog.

Force evaluation is \(O(N \log N)\) and independent per particle once the tree is built. Tree insertion mutates a shared node array, so it stays sequential.

## 2. Parallelisation

| Stage | Strategy | Notes |
| --- | --- | --- |
| Bounding box | `#pragma omp parallel for` with `min`/`max` reductions | Cheap; included for completeness |
| Tree build | sequential | Shared `_nodes` vector, parent splits |
| Mass / COM | sequential | Bottom-up dependence on children |
| Forces | `#pragma omp parallel for schedule(dynamic, 32)` | Main kernel; work per particle is irregular |
| Leapfrog | `#pragma omp parallel for schedule(static)` | Independent writes to each particle |

The OpenMP backend is selected with `--backend omp`. `--backend seq` uses the same CPU kernels with the OpenMP `if` clause disabled, so sequential vs OpenMP results can be compared without a second code path.

`--verify` runs both backends from the same seed and checks positions and velocities.

## 3. How to run

Release build (required for the numbers below):

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
cd build
./quadtrees --verify --particles 2048 --steps 10 --threads 4 --seed 1
./quadtrees --backend seq --particles 4096 --steps 30 --threads 1 --seed 1
./quadtrees --backend omp --particles 4096 --steps 30 --threads 4 --seed 1
```

Headless mode is used whenever `--steps` or `--verify` is passed (no SFML window).

## 4. Test machine

| Item | Value |
| --- | --- |
| CPU | Intel Xeon (4 cores, 1 thread/core) |
| OS | Ubuntu 24.04, Linux 6.12 |
| Compiler | g++ 13.3, `-O3 -DNDEBUG -fopenmp` |
| OpenMP | 4.5 (libgomp) |
| Date | 29 Oct 2026 (Europe/Warsaw) |

Times are average milliseconds per step after one warmup step. Seed `1`, `dt = 0.01`.

## 5. Results

### 5.1 Correctness

```
verify particles=2048 steps=10 max_pos_err=0.000000e+00 max_vel_err=0.000000e+00
ok
```

Sequential and OpenMP trajectories matched to machine precision for this run.

### 5.2 Thread scaling (N = 4096, 30 steps)

| Threads | Backend | avg step (ms) | forces (ms) | Force speedup | Step speedup |
| ---: | --- | ---: | ---: | ---: | ---: |
| 1 | sequential | 10.212 | 9.400 | 1.00× | 1.00× |
| 1 | OpenMP | 10.216 | 9.384 | 1.00× | 1.00× |
| 2 | OpenMP | 5.657 | 4.826 | 1.95× | 1.81× |
| 4 | OpenMP | 3.442 | 2.575 | 3.65× | 2.97× |

Force evaluation scales close to linearly (3.65× on 4 cores). End-to-end speedup is lower because tree build (~0.73 ms) and mass (~0.07 ms) stay serial.

### 5.3 Problem size (20 steps, 4 OpenMP threads vs sequential)

| N | seq avg (ms) | omp-4 avg (ms) | Step speedup | seq forces (ms) | omp-4 forces (ms) | Force speedup |
| ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 1024 | 1.946 | 0.768 | 2.53× | 1.775 | 0.547 | 3.24× |
| 4096 | 10.131 | 3.512 | 2.88× | 9.331 | 2.629 | 3.55× |
| 16384 | 57.598 | 18.427 | 3.13× | 53.885 | 14.629 | 3.68× |

Larger N improves overall speedup: the serial tree build is \(O(N \log N)\) as well, but the parallel force walk dominates more as N grows.

### 5.4 Time breakdown (N = 16384, 4 threads)

| Stage | Sequential (ms) | OpenMP-4 (ms) |
| --- | ---: | ---: |
| bounds | 0.017 | 0.008 |
| build | 3.329 | 3.424 |
| mass | 0.308 | 0.312 |
| forces | 53.885 | 14.629 |
| update | 0.060 | 0.054 |
| **step** | **57.598** | **18.427** |

## 6. Conclusions

- OpenMP on the Barnes–Hut walk is the right CPU parallelisation for this code: particles only write their own acceleration, the tree is read-only, and `dynamic` scheduling absorbs uneven walk cost.
- About 3.7× force speedup on 4 cores is consistent with tree-walk irregularity and memory bandwidth.
- Further CPU gain would require a parallel tree construction (for example spatial sort + level-by-level build), not more threads on the force loop.
- The OpenCL GPU path remains available when OpenCL is found at configure time (`--backend opencl`).
