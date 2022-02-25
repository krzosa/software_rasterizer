

# Things to do:

- [x] Drawing triangles
- [x] Drawing cubes and lines for testing
- [x] Y up coordinate system, left handed
- [x] Drawing a cube with perspective
- [x] Culling triangles facing away from camera
- [x] Texture mapping
- [x] Basic linear transformations - rotation, translation, scaling
- [x] Bilinear filtering of textures / subpixel precison
- [x] Nearest filtering
- [x] Fix the gaps between triangles (it also improved look of triangle edges)
- [ ] Perspective matrix vs simple perspective
- [x] Perspective correct interpolation
- [x] Depth buffer
- [x] Gamma correct blending - converting to almost linear space
- [x] Alpha blending??
- [x] Premultiplied alpha???
- [x] Merge with base
- [ ] Lightning
  - [x] GLOBAL Ilumination
- [x] LookAt Camera
- [ ] FPS Camera
- [x] Reading OBJ models
- [ ] Reading OBJ .mtl files
- [ ] Reading complex obj models (sponza)
- [ ] Reading PMX files
- [ ] Rendering multiple objects, queue renderer
- [x] Clipping
  - [x] Triagnle rectangle bound clipping
  - [x] A way of culling Z out triangles
    - [x] Simple test z clipping
    - [x] Maybe should clip a triangle on znear zfar plane?
    - [x] Maybe should clip out triangles that are fully z out before draw_triangle
- [ ] Subpixel precision of triangle edges
- [x] Simple profiling tooling
- [ ] Statistics based on profiler data, distribution information
- [x] Find cool profilers - ExtraSleepy, Vtune
- [ ] Optimizations
  - [ ] Inline edge function
  - [ ] Edge function to integer
  - [ ] Use integer bit operations to figure out if plus. (edge1|edge2|edge3)>=0
- [ ] SIMD
- [ ] Multithreading
- [ ]
- [ ] Text rendering
- [ ] Basic UI
- [ ] Gamma correct and alpha blending

# Resources that helped me build the rasterizer (Might be helpful to you too):

* Algorithm I used for triangle rasterization by Juan Pineda: https://www.cs.drexel.edu/~david/Classes/Papers/comp175-06-pineda.pdf
* Fabian Giessen's "Optimizing Software Occlusion Culling": https://fgiesen.wordpress.com/2013/02/17/optimizing-sw-occlusion-culling-index/
* Fabian Giessen's optimized software renderer: https://github.com/rygorous/intel_occlusion_cull/tree/blog/SoftwareOcclusionCulling
* Fabian Giessen's javascript triangle rasterizer: https://gist.github.com/rygorous/2486101
* Fabian Giessen's C++ triangle rasterizer: https://github.com/rygorous/trirast/blob/master/main.cpp
* Joy's Kenneth lectures about computer graphics: https://www.youtube.com/playlist?list=PL_w_qWAQZtAZhtzPI5pkAtcUVgmzdAP8g
* Joy's Kenneth article on clipping: https://import.cdn.thinkific.com/167815/JoyKennethClipping-200905-175314.pdf
* A bunch of helpful notes and links to resources: https://nlguillemot.wordpress.com/2016/07/10/rasterizer-notes/
* Very nice paid course on making a software rasterizer using a scanline method: https://pikuma.com/courses/learn-3d-computer-graphics-programming
* Reference for obj loader: https://github.com/tinyobjloader/tinyobjloader/blob/master/tiny_obj_loader.h
*
*
*

## To read

* http://ce-publications.et.tudelft.nl/publications/1362_hardware_algorithms_for_tilebased_realtime_rendering.pdf
