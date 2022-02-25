/////////////////////////////////////////////////////////////////////////////////////
/// 
/// ### Things to do:
/// 
/// - [x] Drawing triangles
/// - [x] Drawing cubes and lines for testing
/// - [x] Y up coordinate system, left handed
/// - [x] Drawing a cube with perspective
/// - [x] Culling triangles facing away from camera
/// - [x] Texture mapping
/// - [x] Basic linear transformations - rotation, translation, scaling
/// - [x] Bilinear filtering of textures / subpixel precison
/// - [x] Nearest filtering
/// - [x] Fix the gaps between triangles (it also improved look of triangle edges)
/// - [ ] Perspective matrix vs simple perspective
/// - [x] Perspective correct interpolation
/// - [x] Depth buffer 
/// - [x] Gamma correct blending - converting to almost linear space
/// - [x] Alpha blending??
/// - [x] Premultiplied alpha???
/// - [x] Merge with base
/// - [ ] Lightning
///   - [x] GLOBAL Ilumination
/// - [x] LookAt Camera
/// - [ ] FPS Camera
/// - [x] Reading OBJ models
/// - [ ] Reading OBJ .mtl files
/// - [ ] Reading complex obj models (sponza)
/// - [ ] Reading PMX files
/// - [ ] Rendering multiple objects, queue renderer
/// - [x] Clipping
///   - [x] Triagnle rectangle bound clipping
///   - [x] A way of culling Z out triangles 
///     - [x] Simple test z clipping
///     - [x] Maybe should clip a triangle on znear zfar plane?
///     - [x] Maybe should clip out triangles that are fully z out before draw_triangle
/// - [ ] Subpixel precision of triangle edges
/// - [x] Simple profiling tooling
/// - [ ] Statistics based on profiler data, distribution information
/// - [x] Find cool profilers - ExtraSleepy, Vtune
/// - [ ] Optimizations
///   - [ ] Inline edge function
///   - [ ] Edge function to integer
///   - [ ] Use integer bit operations to figure out if plus. (edge1|edge2|edge3)>=0
/// - [ ] SIMD
/// - [ ] Multithreading
/// - [ ]
/// - [ ] Text rendering
/// - [ ] Basic UI
/// - [ ] Gamma correct and alpha blending
/// 
/// ### Resources that helped me build the rasterizer (Might be helpful to you too):
/// 
/// * Algorithm I used for triangle rasterization by Juan Pineda: https://www.cs.drexel.edu/~david/Classes/Papers/comp175-06-pineda.pdf
/// * Fabian Giessen's "Optimizing Software Occlusion Culling": https://fgiesen.wordpress.com/2013/02/17/optimizing-sw-occlusion-culling-index/
/// * Fabian Giessen's optimized software renderer: https://github.com/rygorous/intel_occlusion_cull/tree/blog/SoftwareOcclusionCulling
/// * Fabian Giessen's javascript triangle rasterizer: https://gist.github.com/rygorous/2486101
/// * Fabian Giessen's C++ triangle rasterizer: https://github.com/rygorous/trirast/blob/master/main.cpp
/// * Joy's Kenneth lectures about computer graphics: https://www.youtube.com/playlist?list=PL_w_qWAQZtAZhtzPI5pkAtcUVgmzdAP8g
/// * Joy's Kenneth article on clipping: https://import.cdn.thinkific.com/167815/JoyKennethClipping-200905-175314.pdf
/// * A bunch of helpful notes and links to resources: https://nlguillemot.wordpress.com/2016/07/10/rasterizer-notes/
/// * Very nice paid course on making a software rasterizer using a scanline method: https://pikuma.com/courses/learn-3d-computer-graphics-programming
/// * Reference for obj loader: https://github.com/tinyobjloader/tinyobjloader/blob/master/tiny_obj_loader.h
/// *
/// * 
/// * 
///
/// ### To read
///
/// * http://ce-publications.et.tudelft.nl/publications/1362_hardware_algorithms_for_tilebased_realtime_rendering.pdf



#define _CRT_SECURE_NO_WARNINGS
  #define PREMULTIPLIED_ALPHA_BLENDING 1

#define PLATFORM
#include "kpl.h"
#include "profile.cpp"
#include "math.h"

struct R_Vertex {
  Vec3 pos;
  Vec2 tex;
  Vec3 norm;
};

struct R_Render {
  Mat4 camera;
  Mat4 projection;
};

#include "obj_parser.cpp"
#include "stb_image.h"
#include <float.h>

GLOBAL bool draw_rects = 0;
GLOBAL bool draw_wireframe = 0;

FUNCTION
void draw_rect(Bitmap* dst, F32 X, F32 Y, F32 w, F32 h, U32 color) {
  int max_x = (int)(MIN(X + w, dst->x) + 0.5f);
  int max_y = (int)(MIN(Y + h, dst->y) + 0.5f);
  int min_x = (int)(MAX(0, X) + 0.5f);
  int min_y = (int)(MAX(0, Y) + 0.5f);

  for (int y = min_y; y < max_y; y++) {
    for (int x = min_x; x < max_x; x++) {
      dst->pixels[x + y * dst->x] = color;
    }
  }
}

FUNCTION
F32 edge_function(Vec4 vecp0, Vec4 vecp1, Vec4 p) {
  F32 result = (vecp1.y - vecp0.y) * (p.x - vecp0.x) - (vecp1.x - vecp0.x) * (p.y - vecp0.y);
  return result;
}

FUNCTION
Vec4 srgb_to_almost_linear(Vec4 a) {
  Vec4 result = {a.r*a.r, a.g*a.g, a.b*a.b, a.a};
  return result; // @Note: Linear would be to power of 2.2
}

FUNCTION
Vec4 almost_linear_to_srgb(Vec4 a) {
  Vec4 result = { sqrtf(a.r), sqrtf(a.g), sqrtf(a.b), a.a };
  return result;
}

FUNCTION
  void draw_bitmap(Bitmap* dst, Bitmap* src, Vec2 pos) {
  I64 minx = (I64)(pos.x + 0.5);
  I64 miny = (I64)(pos.y + 0.5);
  I64 maxx = minx + src->x;
  I64 maxy = miny + src->y;
  I64 offsetx = 0;
  I64 offsety = 0;

  if (maxx > dst->x) {
    maxx = dst->x;
  }
  if (maxy > dst->y) {
    maxy = dst->y;
  }
  if (minx < 0) {
    offsetx = -minx;
    minx = 0;
  }
  if (miny < 0) {
    offsety = -miny;
    miny = 0;
  }

  for (I64 y = miny; y < maxy; y++) {
    for (I64 x = minx; x < maxx; x++) {
      I64 tx = x - minx + offsetx;
      I64 ty = y - miny + offsety;
      U32 *dst_pixel = dst->pixels + (x + y * dst->x); 
      U32 *pixel     = src->pixels + (tx + ty * src->x); 
      Vec4 result_color = srgb_to_almost_linear(vec4abgr(*pixel));
      Vec4 dst_color = srgb_to_almost_linear(vec4abgr(*dst_pixel));
      result_color.r = result_color.r + (1-result_color.a) * dst_color.r;
      result_color.g = result_color.g + (1-result_color.a) * dst_color.g;
      result_color.b = result_color.b + (1-result_color.a) * dst_color.b;
      result_color.a = result_color.a + dst_color.a - result_color.a*dst_color.a;
      result_color = almost_linear_to_srgb(result_color);
      U32 color32 = vec4_to_u32abgr(result_color);
      *dst_pixel = color32;
    }
  }
}

FUNCTION 
void draw_triangle_nearest(Bitmap* dst, F32 *depth_buffer, Bitmap *src, F32 light,
                  Vec4 p0,   Vec4 p1,   Vec4 p2,
                  Vec2 tex0, Vec2 tex1, Vec2 tex2) {
  F32 min_x1 = (F32)(MIN(p0.x, MIN(p1.x, p2.x)));
  F32 min_y1 = (F32)(MIN(p0.y, MIN(p1.y, p2.y)));
  F32 max_x1 = (F32)(MAX(p0.x, MAX(p1.x, p2.x)));
  F32 max_y1 = (F32)(MAX(p0.y, MAX(p1.y, p2.y)));
  I64 min_x = (I64)MAX(0, floor(min_x1));
  I64 min_y = (I64)MAX(0, floor(min_y1));
  I64 max_x = (I64)MIN(dst->x, ceil(max_x1));
  I64 max_y = (I64)MIN(dst->y, ceil(max_y1));

  F32 area = edge_function(p0, p1, p2);
  for (I64 y = min_y; y < max_y; y++) {
    for (I64 x = min_x; x < max_x; x++) {
      F32 edge1 = edge_function(p0, p1, { (F32)x,(F32)y });
      F32 edge2 = edge_function(p1, p2, { (F32)x,(F32)y });
      F32 edge3 = edge_function(p2, p0, { (F32)x,(F32)y });

      if (edge1 >= 0 && edge2 >= 0 && edge3 >= 0) {
        F32 w1 = edge2 / area;
        F32 w2 = edge3 / area;
        F32 w3 = edge1 / area;
        F32 interpolated_w = (1.f / p0.w) * w1 + (1.f / p1.w) * w2 + (1.f / p2.w) * w3;

        F32 u = tex0.x * (w1 / p0.w) + tex1.x * (w2 / p1.w) + tex2.x * (w3 / p2.w);
        F32 v = tex0.y * (w1 / p0.w) + tex1.y * (w2 / p1.w) + tex2.y * (w3 / p2.w);
        u /= interpolated_w;
        v /= interpolated_w;
        // @Note: We could do: interpolated_w = 1.f / interpolated_w to get proper depth
        // but why waste an instruction, the smaller the depth value the farther the object
        F32* depth = depth_buffer + (x + y * dst->x);
        if (*depth < interpolated_w) {
          *depth = interpolated_w;
          u = u * (src->x - 2);
          v = v * (src->y - 2);
          I64 ui = (I64)(u);
          I64 vi = (I64)(v);
          F32 udiff = u - (F32)ui;
          F32 vdiff = v - (F32)vi;
          // Origin UV (0,0) is in bottom left
          U32 *dst_pixel = dst->pixels + (x + y * dst->x);
          U32 *pixel = src->pixels + (ui + (src->y - 1ll - vi) * src->x);

#if PREMULTIPLIED_ALPHA_BLENDING
          Vec4 result_color = srgb_to_almost_linear(vec4abgr(*pixel));
          Vec4 dst_color = srgb_to_almost_linear(vec4abgr(*dst_pixel));
          result_color.r *= light;
          result_color.g *= light;
          result_color.b *= light;
          result_color.r = result_color.r + (1-result_color.a) * dst_color.r;
          result_color.g = result_color.g + (1-result_color.a) * dst_color.g;
          result_color.b = result_color.b + (1-result_color.a) * dst_color.b;
          result_color.a = result_color.a + dst_color.a - result_color.a*dst_color.a;
          result_color = almost_linear_to_srgb(result_color);
          U32 color32 = vec4_to_u32abgr(result_color);
#else
          U32 color32 = *pixel;
#endif

          *dst_pixel = color32;
        }
      }
    }
  }
  if (draw_rects) {
    draw_rect(dst, p0.x-4, p0.y-4, 8,8, 0x00ff0000);
    draw_rect(dst, p1.x-4, p1.y-4, 8,8, 0x0000ff00);
    draw_rect(dst, p2.x-4, p2.y-4, 8,8, 0x000000ff);
  }
}

FUNCTION 
  void draw_triangle_subpixel(Bitmap* dst, F32 *depth_buffer, Bitmap *src, F32 light,
  Vec4 p0,   Vec4 p1,   Vec4 p2,
  Vec2 tex0, Vec2 tex1, Vec2 tex2) {
  F32 min_x1 = (F32)(MIN(p0.x, MIN(p1.x, p2.x)));
  F32 min_y1 = (F32)(MIN(p0.y, MIN(p1.y, p2.y)));
  F32 max_x1 = (F32)(MAX(p0.x, MAX(p1.x, p2.x)));
  F32 max_y1 = (F32)(MAX(p0.y, MAX(p1.y, p2.y)));
  I64 min_x = (I64)MAX(0, floor(min_x1));
  I64 min_y = (I64)MAX(0, floor(min_y1));
  I64 max_x = (I64)MIN(dst->x, ceil(max_x1));
  I64 max_y = (I64)MIN(dst->y, ceil(max_y1));

  F32 area = edge_function(p0, p1, p2);
  for (I64 y = min_y; y < max_y; y++) {
    for (I64 x = min_x; x < max_x; x++) {
      F32 edge1 = edge_function(p0, p1, { (F32)x,(F32)y });
      F32 edge2 = edge_function(p1, p2, { (F32)x,(F32)y });
      F32 edge3 = edge_function(p2, p0, { (F32)x,(F32)y });

      if (edge1 >= 0 && edge2 >= 0 && edge3 >= 0) {
        F32 w1 = edge2 / area;
        F32 w2 = edge3 / area;
        F32 w3 = edge1 / area;
        F32 interpolated_w = (1.f / p0.w) * w1 + (1.f / p1.w) * w2 + (1.f / p2.w) * w3;

        F32 u = tex0.x * (w1 / p0.w) + tex1.x * (w2 / p1.w) + tex2.x * (w3 / p2.w);
        F32 v = tex0.y * (w1 / p0.w) + tex1.y * (w2 / p1.w) + tex2.y * (w3 / p2.w);
        u /= interpolated_w;
        v /= interpolated_w;
        // @Note: We could do: interpolated_w = 1.f / interpolated_w to get proper depth
        // but why waste an instruction, the smaller the depth value the farther the object
        F32* depth = depth_buffer + (x + y * dst->x);
        if (*depth < interpolated_w && interpolated_w > 0.1f) {
          *depth = interpolated_w;
          u = u * (src->x - 2);
          v = v * (src->y - 2);
          I64 ui = (I64)(u);
          I64 vi = (I64)(v);
          F32 udiff = u - (F32)ui;
          F32 vdiff = v - (F32)vi;
          // Origin UV (0,0) is in bottom left
          U32 *pixel = src->pixels + (ui + (src->y - 1ll - vi) * src->x);
          U32 *dst_pixel = dst->pixels + (x + y * dst->x);

          Vec4 pixelx1y1 = vec4abgr(*pixel);
          Vec4 pixelx2y1 = vec4abgr(*(pixel + 1));
          Vec4 pixelx1y2 = vec4abgr(*(pixel - src->x));
          Vec4 pixelx2y2 = vec4abgr(*(pixel + 1 - src->x));
          pixelx1y1 = srgb_to_almost_linear(pixelx1y1);
          pixelx2y1 = srgb_to_almost_linear(pixelx2y1);
          pixelx1y2 = srgb_to_almost_linear(pixelx1y2);
          pixelx2y2 = srgb_to_almost_linear(pixelx2y2);
          Vec4 blendx1 = lerp(pixelx1y1, pixelx2y1, udiff);
          Vec4 blendx2 = lerp(pixelx1y2, pixelx2y2, udiff);
          Vec4 result_color = lerp(blendx1, blendx2, vdiff);
          result_color.r *= light;
          result_color.g *= light;
          result_color.b *= light;
#if PREMULTIPLIED_ALPHA_BLENDING
          Vec4 dst_color = vec4abgr(*dst_pixel);
          dst_color = srgb_to_almost_linear(dst_color);
          result_color.r = result_color.r + (1-result_color.a) * dst_color.r;
          result_color.g = result_color.g + (1-result_color.a) * dst_color.g;
          result_color.b = result_color.b + (1-result_color.a) * dst_color.b;
          result_color.a = result_color.a + dst_color.a - result_color.a*dst_color.a;
#endif // PREMULTIPLIED_ALPHA_BLENDING
          result_color = almost_linear_to_srgb(result_color);
          U32 color32 = vec4_to_u32abgr(result_color);

          *dst_pixel = color32;
        }
      }
    }
  }
  if (draw_rects) {
    draw_rect(dst, p0.x-4, p0.y-4, 8,8, 0x00ff0000);
    draw_rect(dst, p1.x-4, p1.y-4, 8,8, 0x0000ff00);
    draw_rect(dst, p2.x-4, p2.y-4, 8,8, 0x000000ff);
  }
}

FUNCTION
void draw_line(Bitmap *dst, F32 x0, F32 y0, F32 x1, F32 y1) {
  F32 delta_x = (x1 - x0);
  F32 delta_y = (y1 - y0);
  F32 longest_side_length = (ABS(delta_x) >= ABS(delta_y)) ? ABS(delta_x) : ABS(delta_y);
  F32 x_inc = delta_x / (F32)longest_side_length;
  F32 y_inc = delta_y / (F32)longest_side_length;
  F32 current_x = (F32)x0;
  F32 current_y = (F32)y0;
  for (int i = 0; i <= longest_side_length; i++) {
    int x = (int)(current_x + 0.5f);
    int y = (int)(current_y + 0.5f);
    dst->pixels[x + y * dst->x] = 0xffffffff;
    current_x += x_inc;
    current_y += y_inc;
  }
}

FUNCTION
Bitmap load_image(const char* path) {
  int x, y, n;
  unsigned char* data = stbi_load(path, &x, &y, &n, 4);
  Bitmap result = { (U32*)data, x, y };
#if PREMULTIPLIED_ALPHA_BLENDING
  U32 *p = result.pixels;
  for (int Y = 0; Y < y; Y++) {
    for (int X = 0; X < x; X++) {
      Vec4 color = vec4abgr(*p);
      color.r *= color.a;
      color.g *= color.a;
      color.b *= color.a;
      *p++ = vec4_to_u32abgr(color);
    }
  }
#endif
  return result;
}

FN void r_draw_mesh(ObjMesh *mesh) {
}

int main() {
  os.window_size.x = 1920;
  os.window_size.y = 1080;
  os.window_resizable = 1;
  os_init().error_is_fatal();
  S8List list = {};
  string_push(os.frame_arena, &list, LIT("main.cpp"));
  generate_documentation(list, LIT("README.md"));

  Obj obj = load_obj(LIT("assets/sponza/sponza_mini.obj"));
  
  //Obj obj = load_obj(LIT("assets/f22.obj"));
  ObjMesh *mesh = obj.mesh.e + 1;

  F32 speed = 0.01f;
  F32 rotation = 405;
  Vec3 camera_pos = {0,0,-2};
  //Vec3 camera_target = { 300, 200, 0 };
  Vec3 camera_target = { 0, 0, 0 };
  bool lock_camera_flag = 1;
  Vec3* vertices = (Vec3 *)obj.vertices.e;
  Vec2* tex_coords = (Vec2*)obj.texture_coordinates.e;
  Vec3 *normals = (Vec3 *)obj.normals.e;

  Bitmap img = load_image("assets/bricksx64.png");
  int screen_x = 320;
  int screen_y = 180;
  Bitmap screen320 = {(U32 *)PUSH_SIZE(os.perm_arena, screen_x*screen_y*sizeof(U32)), screen_x, screen_y};
  F32* depth320 = (F32 *)PUSH_SIZE(os.perm_arena, sizeof(F32) * screen_x * screen_y);
  while (os_game_loop()) {
    Mat4 perspective = mat4_perspective(60.f, (F32)os.screen->x, (F32)os.screen->y, 0.1f, 1000.f);
    Mat4 camera = mat4_look_at(camera_pos, camera_target, vec3(0, 1, 0));
    U32* p = screen320.pixels;
    for (int y = 0; y < screen320.y; y++) {
      for (int x = 0; x < screen320.x; x++) {
        *p++ = 0x33333333;
      }
    }
    F32* dp = depth320;
    for (int y = 0; y < screen320.y; y++) {
      for (int x = 0; x < screen320.x; x++) {
        *dp++ = -FLT_MAX;
      }
    }

    Mat4 transform = mat4_rotation_z(rotation);
    transform = transform * mat4_rotation_y(rotation);
    if (os.key[Key_Escape].pressed) os_quit();
    if (os.key[Key_O].down) rotation += 0.05f;
    if (os.key[Key_P].down) rotation -= 0.05f;
    if (os.key[Key_F1].pressed) draw_rects = !draw_rects;
    if (os.key[Key_F2].pressed) draw_wireframe = !draw_wireframe;
    if (os.key[Key_A].down) camera_target.x -= speed;
    if (os.key[Key_D].down) camera_target.x += speed;
    if (os.key[Key_W].down) camera_target.y += speed;
    if (os.key[Key_S].down) camera_target.y -= speed;
    if (os.key[Key_R].down) camera_pos.z += speed;
    if (os.key[Key_F].down) camera_pos.z -= speed;
    for (int i = 0; i < mesh->indices.len; i++) {
      ObjIndex *index = mesh->indices.e + i;
      R_Vertex vert[] = {  
        {
          vertices[index->vertex[0] - 1],
          tex_coords[index->tex[0] - 1],
          normals[index->normal[0] - 1],
        },
        {
          vertices[index->vertex[1] - 1],
          tex_coords[index->tex[1] - 1],
          normals[index->normal[1] - 1],
        },
        {
          vertices[index->vertex[2] - 1],
          tex_coords[index->tex[2] - 1],
          normals[index->normal[2] - 1],
        },
      };
      if (lock_camera_flag) {
        camera_pos = vert[0].pos - vec3(0,-100,130);
        camera_target = vert[0].pos;
        camera = mat4_look_at(camera_pos, camera_target, vec3(0, 1, 0));
          lock_camera_flag = 0;
      }

      //@Note: Transform
      for (int j = 0; j < 3; j++) {
        vert[j].pos = transform * vert[j].pos;
      }

      
      Vec3 p0_to_camera = camera_pos - vert[0].pos;
      Vec3 p0_to_p1 = vert[1].pos - vert[0].pos;
      Vec3 p0_to_p2 = vert[2].pos - vert[0].pos;
      Vec3 normal = normalize(cross(p0_to_p1, p0_to_p2));
      Vec3 light_direction = mat4_rotation_y(45) * vec3(0, 0, 1);
      F32 light = -dot(normal, light_direction);
      light = CLAMP(0.05f, light, 1.f);
      if (dot(normal, p0_to_camera) > 0) { //@Note: Backface culling
        // @Note: Zfar
        B32 vertex_is_outside = false;
        Vec3 zfar_normal = vec3(0, 0, -1);
        Vec3 zfar_pos = vec3(0, 0, 1000.f);
        for (I32 j = 0; j < 3; j++) {
          // @Note: Camera
          vert[j].pos = camera * vert[j].pos;
          // @Note: Skip triangle if even one vertex gets outside the clipping plane
          if ((dot(zfar_normal, vert[j].pos - zfar_pos) < 0)) { 
            vertex_is_outside = true;
            break;
          }
        }

        if (vertex_is_outside) {
          continue;
        }
        
        // @Note: Znear, clip triangles to the near clipping plane
        Vec3 znear_normal = vec3(0, 0, 1);
        Vec3 znear_pos = vec3(0, 0, 1.f);

        struct _R_Vertex {
          Vec4 pos;
          Vec2 tex;
        } in[4];
        I32 in_count = 0;

        R_Vertex *prev = vert + 2;
        R_Vertex *curr = vert;
        F32       prev_dot = dot(znear_normal, prev->pos - znear_pos);
        F32       curr_dot = 0;
        for (int j = 0; j < 3; j++) {
          curr_dot = dot(znear_normal, curr->pos - znear_pos);
          if (curr_dot * prev_dot < 0) {
            F32 t = prev_dot / (prev_dot - curr_dot);
            in[in_count].pos = vec4(lerp(prev->pos, curr->pos, t), 1);
            in[in_count++].tex = lerp(prev->tex, curr->tex, t);
          }
          if (curr_dot > 0) {
            in[in_count].pos = vec4(vert[j].pos, 1);
            in[in_count++].tex = vert[j].tex;
          }
          prev = curr++;
          prev_dot = curr_dot;
        }

        if (in_count == 0) {
          continue;
        }

        for(I64 j = 0; j < in_count; j++) {
          //@Note: Perspective
          in[j].pos = perspective * in[j].pos;
          in[j].pos.x = in[j].pos.x / in[j].pos.w;
          in[j].pos.y = in[j].pos.y / in[j].pos.w;
          in[j].pos.z = in[j].pos.z / in[j].pos.w;
          //@Note: To pixel space
          in[j].pos.x *= screen320.x / 2;
          in[j].pos.y *= screen320.y / 2;
          in[j].pos.x += screen320.x / 2;
          in[j].pos.y += screen320.y / 2;
        }

        if(os.frame > 60) PROFILE_BEGIN(draw_triangle);
        draw_triangle_nearest(&screen320, depth320, &img, light, in[0].pos, in[1].pos, in[2].pos, in[0].tex, in[1].tex, in[2].tex);
        if(os.frame > 60) PROFILE_END(draw_triangle);

        if (in_count > 3) {
          if(os.frame > 60) PROFILE_BEGIN(draw_triangle);
          draw_triangle_nearest(&screen320, depth320, &img, light, in[0].pos, in[2].pos, in[3].pos, in[0].tex, in[2].tex, in[3].tex);
          if(os.frame > 60) PROFILE_END(draw_triangle);
        }
        
        
#if 1
        ProfileScope *scope = profile_scopes + ProfileScopeName_draw_triangle;
        LOCAL_PERSIST B32 profile_flag;
        if (!profile_flag && scope->i > 2000) {
          profile_flag = 1;
          F64 sum = 0;
          for (I64 i = 0; i < profile_scopes[ProfileScopeName_draw_triangle].i; i++) {
            sum += scope->samples[i];
          }
          F64 avg = sum / scope->i;
          S8 data = string_format(os.frame_arena, "avg:%f\n", avg);
          os_append_file(LIT("data.txt"), data);
        }
#endif
        
        if (draw_wireframe) {
          draw_line(&screen320, vert[0].pos.x, vert[0].pos.y, vert[1].pos.x, vert[1].pos.y);
          draw_line(&screen320, vert[1].pos.x, vert[1].pos.y, vert[2].pos.x, vert[2].pos.y);
          draw_line(&screen320, vert[2].pos.x, vert[2].pos.y, vert[0].pos.x, vert[0].pos.y);
        }
      }
    }
    

    // @Note: Draw 320screen to OS screen
    U32* ptr = os.screen->pixels;
    for (int y = 0; y < os.screen->y; y++) {
      for (int x = 0; x < os.screen->x; x++) {
        F32 u = (F32)x / (F32)os.screen->x;
        F32 v = (F32)y / (F32)os.screen->y;
        int tx = (int)(u * screen320.x );
        int ty = (int)(v * screen320.y );
        *ptr++ = screen320.pixels[tx + ty * (screen320.x)];
      }
    }
  }
}
