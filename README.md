# quadtrees
N-body simulation. Other branches may contain (if I haven’t deleted them then experimental and overengeneered) a quadtree data structure with the ability to update particle ownership without rebuilding the whole tree.

## features
- Barnes-Hut algorithm implementation
- Leapfrog integration
- quadtree visualization (SFML)
- simple multithreading (seperate barnach)

## preview
<p align="center">
<img src="https://github.com/user-attachments/assets/1138bcb9-2cab-432b-afb0-1a53b9b5f116" width=300>

<img src="https://github.com/user-attachments/assets/f4974be8-922e-4953-bed3-422cc8ed4134" width=300>

<img src="https://github.com/user-attachments/assets/29dbb7b3-954c-41f4-94b6-67001e117b2b" width=300>
</p>

## todo
- gpu utilization (OpenCL / webGPU / openMP)
- visualization improvements:
  - OpenGL
  - particle trails
  - density based
  - intuitive zoomout 
- exploration of different integration methods (rk4)
- colisions

## resources
- [The Barnes-Hut Algorithm](https://arborjs.org/docs/barnes-hut)
- [An Efficient CUDA Implementation of the Tree-Based Barnes Hut n-Body Algorithm](https://cs.wmich.edu/gupta/teaching/cs5260/5260Sp15web/studentProjects/white+loftus%20nBodySimulations/nbody-problem.pdf)
- [Leapfrog method and other “symplectic” algorithms for integrating Newton’s laws of motion](https://courses.physics.ucsd.edu/2019/Winter/physics141/Assignments/leapfrog.pdf)
