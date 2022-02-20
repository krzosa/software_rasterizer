/* Things to do:
OK Drawing triangles
OK Drawing cubes and lines for testing
OK Y up coordinate system, left handed
OK Drawing a cube with perspective
OK Culling triangles facing away from camera
OK Texture mapping
? Basic math operations on Vec4 Mat4 - Muls, dot, cross etc. 
OK Basic linear transformations - rotation, translation, scaling
OK Bilinear filtering of textures / subpixel precison
OK Fix the gaps between triangles (it also improved look of triangle edges)
* Perspective matrix vs simple perspective
OK Perspective correct interpolation
OK Depth buffer 
KINDA_OK Gamma correct blending
* Alpha blending??
* Premultiplied alpha???
* Lightning
* LookAt Camera
* FPS Camera
OK Reading OBJ files
* Reading PMX files
* Rendering multiple objects, queue renderer
* Clipping
* Optimizations
* SIMD
* Multithreading
*
* Text rendering
* Basic UI
* Gamma correct and alpha blending
*/

/* What a codebase needs:
* Macros for debug, release, slow, fast builds, where debug - tooling, slow - enable asserts
* Macros for OS, Compiler, Architecture
* Nice way of outputing visible error messages
* FatalError and Assert function for release builds with an error message, Debug Assert with error message for slow builds
* 
*/
#define OS_WINDOWS 1
#define PERSPECTIVE_CORRECT_INTERPOLATION 1
#define BILINEAR_BLEND 1
  #define GAMMA_CORRECT_BLENDING 1
  #define PREMULTIPLIED_ALPHA_BLENDING 1

#define _CRT_SECURE_NO_WARNINGS
#include "main.h"
#include "platform.h"
#include "math.h"
#include "stb_image.h"
#include "objparser.h"
#include <float.h>

GLOBAL OS os = {};
GLOBAL bool draw_rects = 0;
GLOBAL bool draw_wireframe = 0;

struct Face { 
  int p[3]; 
  Vec2 tex[3];
};

GLOBAL Vec3 cube_vertices[] = {
  {-1, -1,-1},
  {-1, 1, -1},
  {1, 1,  -1},
  {1, -1, -1},
  {1, 1,  1},
  {1, -1, 1},
  {-1, 1, 1},
  {-1, -1,1},
};

GLOBAL Face cube_faces[] = {
  {{1, 2, 3}, {{ 0, 0 }, { 0, 1 }, { 1, 1 }}, },
  {{1, 3, 4}, {{ 0, 0 }, { 1, 1 }, { 1, 0 }}, },
  {{4, 3, 5}, {{ 0, 0 }, { 0, 1 }, { 1, 1 }}, },
  {{4, 5, 6}, {{ 0, 0 }, { 1, 1 }, { 1, 0 }}, },
  {{6, 5, 7}, {{ 0, 0 }, { 0, 1 }, { 1, 1 }}, },
  {{6, 7, 8}, {{ 0, 0 }, { 1, 1 }, { 1, 0 }}, },
  {{8, 7, 2}, {{ 0, 0 }, { 0, 1 }, { 1, 1 }}, },
  {{8, 2, 1}, {{ 0, 0 }, { 1, 1 }, { 1, 0 }}, },
  {{2, 7, 5}, {{ 0, 0 }, { 0, 1 }, { 1, 1 }}, },
  {{2, 5, 3}, {{ 0, 0 }, { 1, 1 }, { 1, 0 }}, },
  {{6, 8, 1}, {{ 0, 0 }, { 0, 1 }, { 1, 1 }}, }, 
  {{6, 1, 4}, {{ 0, 0 }, { 1, 1 }, { 1, 0 }}, }
};

FUNCTION
void draw_rect(Image* dst, float X, float Y, float w, float h, U32 color) {
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
void draw_bitmap(Image* dst, Image* src, Vec2 pos) {
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
      dst->pixels[x + y * dst->x] = src->pixels[tx + ty * src->x];
    }
  }
}

FUNCTION
float edge_function(Vec4 vecp0, Vec4 vecp1, Vec4 p) {
  float result = (vecp1.y - vecp0.y) * (p.x - vecp0.x) - (vecp1.x - vecp0.x) * (p.y - vecp0.y);
  return result;
}

FUNCTION
Vec4 srgb_to_almost_linear(Vec4 a) {
  Vec4 result = {a.r*a.r, a.g*a.g, a.b*a.b, a.a};
  return result; // @Note: Linear would be to power of 2.2
}

FUNCTION
Vec4 almost_linear_to_srgb(Vec4 a) {
  Vec4 result = { sqrt(a.r), sqrt(a.g), sqrt(a.b), a.a };
  return result;
}

FUNCTION 
void draw_triangle(Image* dst, float *depth_buffer, Image *src, 
                  Vec4 p0,   Vec4 p1,   Vec4 p2,
                  Vec2 tex0, Vec2 tex1, Vec2 tex2) {
  float min_x1 = (float)(MIN(p0.x, MIN(p1.x, p2.x)));
  float min_y1 = (float)(MIN(p0.y, MIN(p1.y, p2.y)));
  float max_x1 = (float)(MAX(p0.x, MAX(p1.x, p2.x)));
  float max_y1 = (float)(MAX(p0.y, MAX(p1.y, p2.y)));
  I64 min_x = (I64)MAX(0, floor(min_x1));
  I64 min_y = (I64)MAX(0, floor(min_y1));
  I64 max_x = (I64)MIN(dst->x, ceil(max_x1));
  I64 max_y = (I64)MIN(dst->y, ceil(max_y1));

  float area = edge_function(p0, p1, p2);
  for (I64 y = min_y; y < max_y; y++) {
    for (I64 x = min_x; x < max_x; x++) {
      float edge1 = edge_function(p0, p1, { (float)x,(float)y });
      float edge2 = edge_function(p1, p2, { (float)x,(float)y });
      float edge3 = edge_function(p2, p0, { (float)x,(float)y });

      if (edge1 >= 0 && edge2 >= 0 && edge3 >= 0) {
        float w1 = edge2 / area;
        float w2 = edge3 / area;
        float w3 = edge1 / area;
        float interpolated_z = (1.f / p0.w) * w1 + (1.f / p1.w) * w2 + (1.f / p2.w) * w3;
#if PERSPECTIVE_CORRECT_INTERPOLATION
        float u = tex0.x * (w1 / p0.w) + tex1.x * (w2 / p1.w) + tex2.x * (w3 / p2.w);
        float v = tex0.y * (w1 / p0.w) + tex1.y * (w2 / p1.w) + tex2.y * (w3 / p2.w);
        u /= interpolated_z;
        v /= interpolated_z;
#else
        float u = tex0.x * w1 + tex1.x * w2 + tex2.x * w3;
        float v = tex0.y * w1 + tex1.y * w2 + tex2.y * w3;
#endif
        // @Note: We could do: interpolated_z = 1.f / interpolated_z to get proper depth
        // but why waste an instruction, the smaller the depth value the farther the object
        float* depth = depth_buffer + (x + y * dst->x);
        if (*depth < interpolated_z) {
          *depth = interpolated_z;
          u = u * (src->x - 2);
          v = v * (src->y - 2);
          I64 ui = (I64)(u);
          I64 vi = (I64)(v);
          float udiff = u - (float)ui;
          float vdiff = v - (float)vi;
          // Origin UV (0,0) is in bottom left
          U32 *pixel = src->pixels + (ui + (src->y - 1ll - vi) * src->x);
          U32 *dst_pixel = dst->pixels + (x + y * dst->x);

#if BILINEAR_BLEND
          Vec4 pixelx1y1 = vec4abgr(*pixel);
          Vec4 pixelx2y1 = vec4abgr(*(pixel + 1));
          Vec4 pixelx1y2 = vec4abgr(*(pixel - src->x));
          Vec4 pixelx2y2 = vec4abgr(*(pixel + 1 - src->x));
  #if GAMMA_CORRECT_BLENDING
          pixelx1y1 = srgb_to_almost_linear(pixelx1y1);
          pixelx2y1 = srgb_to_almost_linear(pixelx2y1);
          pixelx1y2 = srgb_to_almost_linear(pixelx1y2);
          pixelx2y2 = srgb_to_almost_linear(pixelx2y2);
  #endif // GAMMA_CORRECT_BLENDING
          Vec4 blendx1 = lerp(pixelx1y1, pixelx2y1, udiff);
          Vec4 blendx2 = lerp(pixelx1y2, pixelx2y2, udiff);
          Vec4 result_color = lerp(blendx1, blendx2, vdiff);
  #if PREMULTIPLIED_ALPHA_BLENDING
          Vec4 dst_color = vec4abgr(*dst_pixel);
    #if GAMMA_CORRECT_BLENDING
          dst_color = srgb_to_almost_linear(dst_color);
    #endif
          result_color.r = result_color.r + (1-result_color.a) * dst_color.r;
          result_color.g = result_color.g + (1-result_color.a) * dst_color.g;
          result_color.b = result_color.b + (1-result_color.a) * dst_color.b;
          result_color.a = result_color.a + dst_color.a - result_color.a*dst_color.a;
  #endif // PREMULTIPLIED_ALPHA_BLENDING
  #if GAMMA_CORRECT_BLENDING
          result_color = almost_linear_to_srgb(result_color);
          ASSERT(result_color.r <= 1 && result_color.g <= 1 && result_color.b <= 1);
  #endif // GAMMA_CORRECT_BLENDING
          U32 color32 = color_to_u32abgr(result_color);
#else // BILINEAR_BLEND
          Vec4 result_color = srgb_to_almost_linear(vec4abgr(*pixel));
          Vec4 dst_color = srgb_to_almost_linear(vec4abgr(*dst_pixel));
          result_color.r = result_color.r + (1-result_color.a) * dst_color.r;
          result_color.g = result_color.g + (1-result_color.a) * dst_color.g;
          result_color.b = result_color.b + (1-result_color.a) * dst_color.b;
          result_color.a = result_color.a + dst_color.a - result_color.a*dst_color.a;
          result_color = almost_linear_to_srgb(result_color);
          U32 color32 = color_to_u32abgr(result_color);
#endif // BILINEAR_BLEND

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
void draw_line(Image *dst, float x0, float y0, float x1, float y1) {
  float delta_x = (x1 - x0);
  float delta_y = (y1 - y0);
  float longest_side_length = (ABS(delta_x) >= ABS(delta_y)) ? ABS(delta_x) : ABS(delta_y);
  float x_inc = delta_x / (float)longest_side_length;
  float y_inc = delta_y / (float)longest_side_length;
  float current_x = (float)x0;
  float current_y = (float)y0;
  for (int i = 0; i <= longest_side_length; i++) {
    int x = (int)(current_x + 0.5f);
    int y = (int)(current_y + 0.5f);
    dst->pixels[x + y * dst->x] = 0xffffffff;
    current_x += x_inc;
    current_y += y_inc;
  }
}

struct FaceA {
  int vertex[3];
  int tex[3];
  int normal[3];
};

FUNCTION
Obj load_obj(const char* file) {
  char* data = os.read_file(file);
  char* memory = (char*)malloc(100000);
  Obj result = obj::parse(memory, 100000, data);
  free(data);
  return result;
}

FUNCTION
Image load_image(const char* path) {
  int x, y, n;
  unsigned char* data = stbi_load(path, &x, &y, &n, 4);
  Image result = { (U32*)data, x, y };
#if PREMULTIPLIED_ALPHA_BLENDING
  U32 *p = result.pixels;
  for (int Y = 0; Y < y; Y++) {
    for (int X = 0; X < x; X++) {
      Vec4 color = vec4abgr(*p);
      color.r *= color.a;
      color.g *= color.a;
      color.b *= color.a;
      *p++ = color_to_u32abgr(color);
    }
  }
#endif
  return result;
}

int main() {
  obj::test();
  os.init({ 1280,720 });

  float rotation = 0;
  Vec3 camera_pos = {0,0,-5};
  
  Obj obj = load_obj("assets/cube.obj");
  Vec3* vertices = (Vec3 *)obj.vertices;
  Vec2* tex_coords = (Vec2*)obj.texture;
  FaceA* faces = (FaceA*)obj.indices;
  I64 face_count = obj.indices_count;


  Image img = load_image("assets/cat.png");
  int screen_x = 160;
  int screen_y = 90;
  Image screen320 = {(U32 *)malloc(screen_x*screen_y*sizeof(U32)), screen_x, screen_y};
  float* depth320 = (float *)malloc(sizeof(float) * screen_x * screen_y);
  while (os.game_loop()) {
    Mat4 perspective = make_matrix_perspective(60.f, (float)os.screen.x, (float)os.screen.y, 0.1f, 100.f);
    U32* p = screen320.pixels;
    for (int y = 0; y < screen320.y; y++) {
      for (int x = 0; x < screen320.x; x++) {
        *p++ = 0x44444444;
      }
    }
    float* dp = depth320;
    for (int y = 0; y < screen320.y; y++) {
      for (int x = 0; x < screen320.x; x++) {
        *dp++ = -FLT_MAX;
      }
    }
//draw_bitmap(&screen320, &img, {0,0});
    Mat4 transform = make_matrix_rotation_z(rotation);
    transform = transform * make_matrix_rotation_x(rotation);
    if (os.keydown_a) rotation += 0.05f;
    if (os.keydown_b) rotation -= 0.05f;
    if (os.keydown_f1) draw_rects = !draw_rects;
    if (os.keydown_f2) draw_wireframe = !draw_wireframe;
    for (int i = 0; i < face_count; i++) {
      FaceA* face = faces + i;
      Vec4 pos[3] = {
        vec4(vertices[face->vertex[0] - 1], 1),
        vec4(vertices[face->vertex[1] - 1], 1),
        vec4(vertices[face->vertex[2] - 1], 1),
      };
      Vec2 tex[3] = {
        tex_coords[face->tex[0] - 1],
        tex_coords[face->tex[1] - 1],
        tex_coords[face->tex[2] - 1],
      };

      //@Note: Transform
      for (int j = 0; j < 3; j++) {
        pos[j] = transform * pos[j];
      }
      //@Note: Cull
      Vec3 p0_to_camera = camera_pos - pos[0].xyz;
      Vec3 p0_to_p1 = pos[1].xyz - pos[0].xyz;
      Vec3 p0_to_p2 = pos[2].xyz - pos[0].xyz;
      Vec3 normal = cross(p0_to_p1, p0_to_p2);
      if (dot(normal, p0_to_camera) > 0) {
        for (int j = 0; j < 3; j++) {
          //@Note: Camera
          pos[j].xyz = pos[j].xyz - camera_pos;
          //@Note: Perspective
          pos[j] = perspective * pos[j];
          pos[j].x = pos[j].x / pos[j].w;
          pos[j].y = pos[j].y / pos[j].w;
          pos[j].z = pos[j].z / pos[j].w;
          //@Note: To pixel space
          pos[j].x *= screen320.x / 2;
          pos[j].y *= screen320.y / 2;
          pos[j].x += screen320.x / 2;
          pos[j].y += screen320.y / 2;
        }

        draw_triangle(&screen320, depth320, &img, pos[0], pos[1], pos[2], tex[0], tex[1], tex[2]);
        for (int j = 0; j < 3; j++) {
          pos[j].x += screen320.x / 8;
          pos[j].y += screen320.y / 8;
        }
        draw_triangle(&screen320, depth320, &img, pos[0], pos[1], pos[2], tex[0], tex[1], tex[2]);
        if (draw_wireframe) {
          draw_line(&screen320, pos[0].x, pos[0].y, pos[1].x, pos[1].y);
          draw_line(&screen320, pos[1].x, pos[1].y, pos[2].x, pos[2].y);
          draw_line(&screen320, pos[2].x, pos[2].y, pos[0].x, pos[0].y);
        }
      }
    }

    // @Note: Draw 320screen to OS screen
    U32* ptr = os.screen.pixels;
    for (int y = 0; y < os.screen.y; y++) {
      for (int x = 0; x < os.screen.x; x++) {
        float u = (float)x / (float)os.screen.x;
        float v = (float)y / (float)os.screen.y;
        int tx = (int)(u * screen320.x + 0.5f);
        int ty = (int)(v * screen320.y + 0.5f);
        *ptr++ = screen320.pixels[tx + ty * (screen320.x)];
      }
    }
  }
}
