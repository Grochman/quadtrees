# quadtrees
N-body simulation. Other branches may contain (if I haven’t deleted them then experimental and overengeneered) a quadtree data structure with the ability to update particle ownership without rebuilding the whole tree.

## features
- Barnes-Hut algorithm implementation
- Leapfrog integration
- quadtree visualization (SFML)
- simple multithreading (seperate barnach)

## preview
![QuadTrees 2025-07-20 11-50-33](https://github.com/user-attachments/assets/8b4804a8-306c-4aad-b8e9-d5664dc895a6)



## todo
- gpu utilization (OpenCL / SYCL)
- visualization with OpenGL (better performance plus SFML does not handle transparency well - no easy way to draw fading particle trails)
- density based visualization
- exploration of different integration methods (rk4)

## resources
- https://arborjs.org/docs/barnes-hut
- https://courses.physics.ucsd.edu/2019/Winter/physics141/Assignments/leapfrog.pdf
