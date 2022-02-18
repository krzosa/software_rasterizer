/* Things to do:
OK Drawing triangles
OK Drawing cubes and lines for testing
OK Y up coordinate system, left handed
OK Drawing a cube with perspective
OK Culling triangles facing away from camera
OK Texture mapping
? Basic math operations on Vec4 Mat4 - Muls, Dot, Cross etc. 
OK Basic linear transformations - rotation, translation, scaling
OK Bilinear filtering of textures / subpixel precison
OK Fix the gaps between triangles (it also improved look of triangle edges)
* Perspective matrix vs simple perspective
* Perspective correct interpolation
* Depth buffer 
* FPS Camera
* Reading OBJ files
* Loading a model from PMX?
* Rendering multiple objects, queue renderer
* Clipping
* Optimizations
* SIMD
* Multithreading
*
*/

/* What a codebase needs:
* Macros for debug, release, slow, fast builds, where debug - tooling, slow - enable asserts
* Macros for OS, Compiler, Architecture
* Nice way of outputing visible error messages
* FatalError and Assert function for release builds with an error message, Debug Assert with error message for slow builds
* 
*/
#define OS_WINDOWS 1
#define BILINEAR_BLEND 1
#define PERSPECTIVE_CORRECT_INTERPOLATION 1
#define DRAW_WIREFRAME 1
#define DRAW_RECTS 0

#include "main.h"
#include "platform.h"
#include "math.h"
#include "stb_image.h"
#include "objparser.h"

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
void DrawRect(Image* dst, float X, float Y, float w, float h, U32 color) {
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
void DrawBitmap(Image* dst, Image* src, Vec2 pos) {
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
float EdgeFunction(Vec4 vecp0, Vec4 vecp1, Vec4 p) {
  float result = (vecp1.y - vecp0.y) * (p.x - vecp0.x) - (vecp1.x - vecp0.x) * (p.y - vecp0.y);
  return result;
}

FUNCTION 
void DrawTriangle(Image* dst, Image *src, Vec4 p0, Vec4 p1, Vec4 p2,
                  Vec2 tex0, Vec2 tex1, Vec2 tex2) {
  float min_x1 = (float)(MIN(p0.x, MIN(p1.x, p2.x)));
  float min_y1 = (float)(MIN(p0.y, MIN(p1.y, p2.y)));
  float max_x1 = (float)(MAX(p0.x, MAX(p1.x, p2.x)));
  float max_y1 = (float)(MAX(p0.y, MAX(p1.y, p2.y)));
  I64 min_x = (I64)MAX(0, Floor(min_x1));
  I64 min_y = (I64)MAX(0, Floor(min_y1));
  I64 max_x = (I64)MIN(dst->x, Ceil(max_x1));
  I64 max_y = (I64)MIN(dst->y, Ceil(max_y1));

  float area = EdgeFunction(p0, p1, p2);
  for (I64 y = min_y; y < max_y; y++) {
    for (I64 x = min_x; x < max_x; x++) {
      float edge1 = EdgeFunction(p0, p1, { (float)x,(float)y });
      float edge2 = EdgeFunction(p1, p2, { (float)x,(float)y });
      float edge3 = EdgeFunction(p2, p0, { (float)x,(float)y });

      if (edge1 >= 0 && edge2 >= 0 && edge3 >= 0) {
        float w1 = edge2 / area;
        float w2 = edge3 / area;
        float w3 = edge1 / area;
#if PERSPECTIVE_CORRECT_INTERPOLATION
        float u = tex0.x * (w1 / p0.w) + tex1.x * (w2 / p1.w) + tex2.x * (w3 / p2.w);
        float v = tex0.y * (w1 / p0.w) + tex1.y * (w2 / p1.w) + tex2.y * (w3 / p2.w);

        float interpolated_z = (1.f / p0.w) * w1 + (1.f / p1.w) * w2 + (1.f / p2.w) * w3;
        u /= interpolated_z;
        v /= interpolated_z;
#else
        float u = tex0.x * w1 + tex1.x * w2 + tex2.x * w3;
        float v = tex0.y * w1 + tex1.y * w2 + tex2.y * w3;
#endif
        u = u * (src->x - 2);
        v = v * (src->y - 2);
        I64 ui = (I64)(u);
        I64 vi = (I64)(v);
        float udiff = u - (float)ui;
        float vdiff = v - (float)vi;
        // Origin UV (0,0) is in bottom left
        U32 *pixel = src->pixels + (ui + (src->y - 1ll - vi) * src->x);
#if BILINEAR_BLEND
        Vec4 pixelx1y1 = V4ABGR(*pixel);
        Vec4 pixelx2y1 = V4ABGR(*(pixel + 1));
        Vec4 pixelx1y2 = V4ABGR(*(pixel - src->x));
        Vec4 pixelx2y2 = V4ABGR(*(pixel + 1 - src->x));

        Vec4 blendx1 = Lerp(pixelx1y1, pixelx2y1, udiff);
        Vec4 blendx2 = Lerp(pixelx1y2, pixelx2y2, udiff);
        Vec4 result_color = Lerp(blendx1, blendx2, vdiff);
        U32 color32 = ColorToU32ABGR(result_color);
#else
        U32 color32 = *pixel;
#endif
        
        dst->pixels[x + y * dst->x] = color32;
      }
    }
  }
#if DRAW_RECTS
  DrawRect(dst, p0.x-4, p0.y-4, 8,8, 0x00ff0000);
  DrawRect(dst, p1.x-4, p1.y-4, 8,8, 0x0000ff00);
  DrawRect(dst, p2.x-4, p2.y-4, 8,8, 0x000000ff);
#endif
}

FUNCTION
void DrawLine(Image *dst, float x0, float y0, float x1, float y1) {
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
Obj LoadObj(const char* file) {
  char* data = OS_ReadFile(file);
  char* memory = (char*)malloc(100000);
  Obj result = Obj_Parse(memory, 100000, data);
  free(data);
  return result;
}

FUNCTION
Image LoadImage(const char* path) {
  int x, y, n;
  unsigned char* data = stbi_load(path, &x, &y, &n, 4);
  Image result = { (U32*)data, x, y };
  return result;
}

int main() {
  Obj_Test();
  OS_Init({ 1280,720 });
  float rotation = 0;
  Vec3 camera_pos = {0,0,-5};
  
  Obj obj = LoadObj("assets/f22.obj");
  Vec3* vertices = (Vec3 *)obj.vertices;
  Vec2* tex_coords = (Vec2*)obj.texture;
  FaceA* faces = (FaceA*)obj.indices;
  I64 face_count = obj.indices_count;


  Image img = LoadImage("assets/bricksx64.png");
  Image screen320 = {(U32 *)malloc(320*180*sizeof(U32)), 320, 180};
  while (OS_GameLoop()) {
    Mat4 perspective = Mat4Perspective(60.f, (float)screen.x, (float)screen.y, 0.1f, 100.f);
    for (int y = 0; y < screen320.y; y++) {
      for (int x = 0; x < screen320.x; x++) {
        screen320.pixels[x + y * screen320.x] = 0;
      }
    }
    DrawBitmap(&screen320, &img, {0,0});
    Mat4 transform = Mat4RotationZ(rotation);
    transform = transform * Mat4RotationX(rotation);
    if (keydown_a) rotation += 0.05f;
    if (keydown_b) rotation -= 0.05f;
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
      Vec3 normal = Cross(p0_to_p1, p0_to_p2);
      if (Dot(normal, p0_to_camera) > 0) {
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

        DrawTriangle(&screen320, &img, pos[0], pos[1], pos[2], tex[0], tex[1], tex[2]);
#if DRAW_WIREFRAME
        DrawLine(&screen320, pos[0].x, pos[0].y, pos[1].x, pos[1].y);
        DrawLine(&screen320, pos[1].x, pos[1].y, pos[2].x, pos[2].y);
        DrawLine(&screen320, pos[2].x, pos[2].y, pos[0].x, pos[0].y);
#endif
      }
    }

    // @Note: Draw 320screen to OS screen
    U32* ptr = screen.pixels;
    for (int y = 0; y < screen.y; y++) {
      for (int x = 0; x < screen.x; x++) {
        float u = (float)x / (float)screen.x;
        float v = (float)y / (float)screen.y;
        int tx = (int)(u * screen320.x + 0.5f);
        int ty = (int)(v * screen320.y + 0.5f);
        *ptr++ = screen320.pixels[tx + ty * (screen320.x)];
      }
    }
  }
}
