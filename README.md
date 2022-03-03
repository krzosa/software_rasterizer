

### Things to do:

- [x] Drawing triangles
- [x] Drawing cubes and lines for testing
- [x] Y up coordinate system, left handed
- [x] Drawing a cube with perspective
- [x] Culling triangles facing away from camera
- [x] Texture mapping
- [x] Basic linear transformations - rotation, translation, scaling
- [x] Bilinear filtering of textures
- [x] Nearest filtering
- [x] Fix the gaps between triangles (it also improved look of triangle edges)
- [ ] Perspective matrix vs simple perspective
- [x] Perspective correct interpolation
- [x] Depth buffer
- [x] Gamma correct blending - converting to almost linear space
- [x] Alpha blending
- [x] Premultiplied alpha
- [x] Merge with base
- [ ] Fill convention
- [ ] Antialiasing (seems like performance gets really bad with this)
- [x] LookAt Camera
- [x] FPS Camera
- [ ] Quarternions for rotations
- [x] Reading OBJ models
- [ ] Reading more OBJ formats
- [x] Reading OBJ .mtl files
- [x] Loading materials
- [x] Rendering textures obj models
- [x] Reading complex obj models (sponza)
- [x] Fix sponza uv coordinates - the issue was uv > 1 and uv < 0
- [x] Clipping
  - [x] Triagnle rectangle bound clipping
  - [x] A way of culling Z out triangles
    - [x] Simple test z clipping
    - [x] Maybe should clip a triangle on znear zfar plane?
    - [x] Maybe should clip out triangles that are fully z out before draw_triangle
- [ ] Effects!!!
  - [ ] Outlines
- [ ] Lightning
  - [ ] Proper normal interpolation
    * https://hero.handmade.network/episode/code/day101/#105
  - [ ] Phong
    - [x] diffuse
    - [x] ambient
    - [ ] specular
     * reflecting vectors
  - [ ] Use all materials from OBJ
  - [ ] Point light
- [ ] Reading PMX files
- [ ] Rendering multiple objects, queue renderer
  - [x] Simple function to render a mesh
- [x] Simple profiling tooling
- [x] Statistics based on profiler data
- [x] Find cool profilers - ExtraSleepy, Vtune
- [ ] Optimizations
  - [ ] Inline edge function
  - [ ] Expand edge functions to more optimized version
  - [ ] Test 4x2 bitmap layout?
  - [ ] Edge function to integer
  - [ ] Use integer bit operations to figure out if plus. (edge0|edge1|edge2)>=0
  - [ ] SIMD
  - [ ] Multithreading

- [x] Text rendering
- [ ] UI
  - [x] Labels
  - [x] Settings variables
  - [x] Signals
  - [ ] Sliders
  - [ ] Groups
- [x] Gamma correct alpha blending for rectangles and bitmaps
- [ ] Plotting of profile data
   - [x] Simple scatter plot


## Clipping

There are 3 clipping stages, 2 clipping stages in 3D space against zfar and znear and 1 clipping
stage in 2D against left, bottom, right, top(2D image bounds).

First the triangles get clipped against the zfar plane,
if a triangle has even one vertex outside the clipping region, the entire triangle gets cut.
So far I didn't have problems with that. It simplifies the computations and splitting triangles
on zfar seems like a waste of power.

The second clipping stage is znear plane. Triangles get fully and nicely clipped against znear.
Every time a triangle gets partially outside the clipping region it gets cut to the znear and
either one or two new triangles get derived from the old one.

Last clipping stage is performed in the 2D image space. Every triangle has a corresponding AABB
box. In this box every pixel gets tested to see if it's in the triangle. In this clipping stage
the box is clipped to the image metrics - 0, 0, width, height.



### Resources that helped me build the rasterizer (Might be helpful to you too):

* Algorithm I used for triangle rasterization by Juan Pineda is described in paper called "A Parallel Algorithm for Polygon Rasterization"
* Casey Muratori's series on making a game from scratch(including a 2D software rasterizer(episode ~82) and 3d gpu renderer): https://hero.handmade.network/episode/code#
* Fabian Giessen's "Optimizing Software Occlusion Culling": https://fgiesen.wordpress.com/2013/02/17/optimizing-sw-occlusion-culling-index/
* Fabian Giessen's optimized software renderer: https://github.com/rygorous/intel_occlusion_cull/tree/blog/SoftwareOcclusionCulling
* Fabian Giessen's javascript triangle rasterizer: https://gist.github.com/rygorous/2486101
* Fabian Giessen's C++ triangle rasterizer: https://github.com/rygorous/trirast/blob/master/main.cpp
* Joy's Kenneth lectures about computer graphics: https://www.youtube.com/playlist?list=PL_w_qWAQZtAZhtzPI5pkAtcUVgmzdAP8g
* Joy's Kenneth article on clipping: https://import.cdn.thinkific.com/167815/JoyKennethClipping-200905-175314.pdf
* A bunch of helpful notes and links to resources: https://nlguillemot.wordpress.com/2016/07/10/rasterizer-notes/
* Very nice paid course on making a software rasterizer using a scanline method: https://pikuma.com/courses/learn-3d-computer-graphics-programming
* Reference for obj loader: https://github.com/tinyobjloader/tinyobjloader/blob/master/tiny_obj_loader.h

### To read

* http://ce-publications.et.tudelft.nl/publications/1362_hardware_algorithms_for_tilebased_realtime_rendering.pdf
