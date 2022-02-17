/* Things to do:
OK Drawing triangles
OK Drawing cubes and lines for testing
OK Y up coordinate system, left handed
OK Drawing a cube with perspective
OK Culling triangles facing away from camera
OK Texture mapping
? Basic math operations on Vec4 Mat4 - Muls, Dot, Cross etc. 
OK Basic linear transformations - rotation, translation, scaling
* Bilinear filtering of textures / subpixel precison
* Perspective matrix vs simple perspective
* Perspective correct interpolation
* Depth buffer 
* FPS Camera
* Reading OBJ files
* Rendering multiple objects, queue renderer
* Clipping
* Optimizations
* SIMD
* Multithreading
*
*/

#include "main.h"
#include "platform.h"
#include "math.h"
#include "stb_image.h"

struct Face { 
  int p[3]; 
  Vec2 tex[3];
};

Vec3 cube_vertices[] = {
  {-1, -1,-1},
  {-1, 1, -1},
  {1, 1,  -1},
  {1, -1, -1},
  {1, 1,  1},
  {1, -1, 1},
  {-1, 1, 1},
  {-1, -1,1},
};

Face cube_faces[] = {
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
void DrawRect(Image* dst, float X, float Y, float w, float h, uint32_t color) {
  int max_x = (int)(MIN(X + w, dst->x) + 0.5f);
  int max_y = (int)(MIN(Y + h, dst->y) + 0.5f);
  int min_x = (int)(MAX(0, X) + 0.5f);
  int min_y = (int)(MAX(0, Y) + 0.5f);

  for (int y = min_y; y < max_y; y++) {
    for (int x = min_x; x < max_x; x++) {
      dst->pixels[x + (dst->y - 1 - y) * dst->x] = color;
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
  int min_x = (int)MAX(0, floorf(min_x1));
  int min_y = (int)MAX(0, floorf(min_y1));
  int max_x = (int)MIN(dst->x, ceilf(max_x1));
  int max_y = (int)MIN(dst->y, ceilf(max_y1));
  //@Todo: Fix the proper rounding 
  //@Todo: Determine whether we need subprecision etc

  float area = EdgeFunction(p0, p1, p2);
  for (int y = min_y; y < max_y; y++) {
    for (int x = min_x; x < max_x; x++) {
      float edge1 = EdgeFunction(p0, p1, { (float)x,(float)y });
      float edge2 = EdgeFunction(p1, p2, { (float)x,(float)y });
      float edge3 = EdgeFunction(p2, p0, { (float)x,(float)y });

      if (edge1 >= 0 && edge2 >= 0 && edge3 >= 0) {
        float w1 = edge2 / area;
        float w2 = edge3 / area;
        float w3 = edge1 / area;
#if 1
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
        int ui = (int)(u);
        int vi = (int)(v);
        float udiff = u - (float)ui;
        float vdiff = v - (float)vi;

        uint32_t *pixel = src->pixels + (ui + (src->y - 1 - vi) * src->x);
        Vec4 pixelx1y1 = V4ABGR(*pixel);
        Vec4 pixelx2y1 = V4ABGR(*(pixel + 1));
        Vec4 pixelx1y2 = V4ABGR(*(pixel - src->x));
        Vec4 pixelx2y2 = V4ABGR(*(pixel + 1 - src->x));

        Vec4 blendx1 = Lerp(pixelx1y1, pixelx2y1, udiff);
        Vec4 blendx2 = Lerp(pixelx1y2, pixelx2y2, udiff);
        Vec4 result_color = Lerp(blendx1, blendx2, vdiff);
        uint32_t color32 = ColorToU32ARGB(result_color);
        
        dst->pixels[x + (dst->y - 1 - y) * dst->x] = color32;
      }
    }
  }
  DrawRect(dst, p0.x-4, p0.y-4, 8,8, 0x00ff0000);
  DrawRect(dst, p1.x-4, p1.y-4, 8,8, 0x0000ff00);
  DrawRect(dst, p2.x-4, p2.y-4, 8,8, 0x000000ff);
}

FUNCTION
void DrawLine(Image *dst, float x0, float y0, float x1, float y1) {
  float delta_x = (x1 - x0);
  float delta_y = (y1 - y0);
  float longest_side_length = (fabsf(delta_x) >= fabsf(delta_y)) ? fabsf(delta_x) : fabsf(delta_y);
  float x_inc = delta_x / (float)longest_side_length;
  float y_inc = delta_y / (float)longest_side_length;
  float current_x = (float)x0;
  float current_y = (float)y0;
  for (int i = 0; i <= longest_side_length; i++) {
    int x = (int)(current_x + 0.5f);
    int y = (int)(current_y + 0.5f);
    dst->pixels[x + (dst->y - 1 - y) * dst->x] = 0xffffffff;
    current_x += x_inc;
    current_y += y_inc;
  }
}

int main() {
  OS_Init({ 1280,720 });
  float rotation = 0;
  Vec3 camera_pos = {0,0,-5};
  int x,y,n;
  unsigned char *data = stbi_load("assets/bricksx64.png", &x, &y, &n, 4);
  Image img = {(uint32_t *)data, x, y};
  Mat4 perspective = Mat4Perspective(60.f, screen.x, screen.y, 0.1f, 100.f);
  while (OS_GameLoop()) {
    for (int y = 0; y < screen.y; y++) {
      for (int x = 0; x < screen.x; x++) {
        screen.pixels[x + y * screen.x] = 0;
      }
    }
#if 0
    DrawTriangle(&screen, &img, { 100,100 }, { 100,400 }, { 400,400 }, { 0,0 }, { 0,1 }, { 1,1 });
    DrawTriangle(&screen, &img, { 100,100 }, { 400,400 }, { 400,100}, { 0,0 }, { 1,1 }, { 1,0 });
#else
    Mat4 transform = Mat4RotationZ(rotation);
    transform = transform * Mat4RotationX(rotation);
    if (keydown_a) rotation += 0.05f;
    if (keydown_b) rotation -= 0.05f;
    for (int i = 0; i < ARRAY_CAP(cube_faces); i++) {
      Face* face = cube_faces + i;
      Vec4 pos[3] = {
        vec4(cube_vertices[face->p[0] - 1], 1),
        vec4(cube_vertices[face->p[1] - 1], 1),
        vec4(cube_vertices[face->p[2] - 1], 1),
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
          pos[j].x *= screen.x / 2;
          pos[j].y *= screen.y / 2;
          pos[j].x += screen.x / 2;
          pos[j].y += screen.y / 2;
        }
        DrawTriangle(&screen, &img, pos[0], pos[1], pos[2], face->tex[0], face->tex[1], face->tex[2]);
        //DrawLine(&screen, pos[0].x, pos[0].y, pos[1].x, pos[1].y);
        //DrawLine(&screen, pos[1].x, pos[1].y, pos[2].x, pos[2].y);
        //DrawLine(&screen, pos[2].x, pos[2].y, pos[0].x, pos[0].y);
      }
    }
#endif
  }
}
