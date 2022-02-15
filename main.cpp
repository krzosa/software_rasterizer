#include "main.h"
#include "platform.h"
#include "math.h"
#include "stb_image.h"


// TODO:
// Perspective correct interpolation
// Texture mapping
// Counter clockwise triangle culling
// Reading OBJ

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
float EdgeFunction(Vec3 vecp0, Vec3 vecp1, Vec3 p) {
  float result = (vecp1.y - vecp0.y) * (p.x - vecp0.x) - (vecp1.x - vecp0.x) * (p.y - vecp0.y);
  return result;
}

FUNCTION 
void DrawTriangle(Image* dst, Image *src, Vec3 p0, Vec3 p1, Vec3 p2,
                  Vec2 tex0, Vec2 tex1, Vec2 tex2) {
  float min_x = (float)(MIN(p0.x, MIN(p1.x, p2.x)) + 0.5f);
  float min_y = (float)(MIN(p0.y, MIN(p1.y, p2.y)) + 0.5f);
  float max_x = (float)(MAX(p0.x, MAX(p1.x, p2.x)) + 0.5f);
  float max_y = (float)(MAX(p0.y, MAX(p1.y, p2.y)) + 0.5f);
  min_x = MAX(0, min_x);
  min_y = MAX(0, min_y);
  max_x = MIN(dst->x-1, max_x);
  max_y = MIN(dst->y-1, max_y);

  float area = EdgeFunction(p0, p1, p2);
  for (float y = min_y; y < max_y; y++) {
    for (float x = min_x; x < max_x; x++) {
      float edge1 = EdgeFunction(p0, p1, { x,y });
      float edge2 = EdgeFunction(p1, p2, { x,y });
      float edge3 = EdgeFunction(p2, p0, { x,y });

      if (edge1 >= 0 && edge2 >= 0 && edge3 >= 0) {
        int xi = (int)(x + 0.5f);
        int yi = (int)(y + 0.5f);
        float w1 = edge2 / area;
        float w2 = edge3 / area;
        float w3 = edge1 / area;
        float u = tex0.x * w1 + tex1.x * w2 + tex2.x * w3;
        float v = tex0.y * w1 + tex1.y * w2 + tex2.y * w3;
        int ui = (int)(u * (src->x - 1) + 0.5f);
        int vi = (int)(v * (src->y - 1) + 0.5f);
        uint32_t pixel = src->pixels[ui + (src->y - 1 - vi) * src->x];
        dst->pixels[xi + (dst->y - 1 - yi) * dst->x] = pixel;
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
  OS_Init({.window_x=1280, .window_y=720});
  float rotation = 0;
  Vec3 camera_pos = {0,0,-5};
  int x,y,n;
  unsigned char *data = stbi_load("assets/cat.png", &x, &y, &n, 4);
  Image img = {
    (uint32_t *)data, x, y
  };
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
      Vec3 pos[3] = {
        cube_vertices[face->p[0] - 1],
        cube_vertices[face->p[1] - 1],
        cube_vertices[face->p[2] - 1],
      };

      //@Note: Transform
      for (int j = 0; j < 3; j++) {
        pos[j] = transform * pos[j];
      }
      //@Note: Cull
      Vec3 p0_to_camera = camera_pos - pos[0];
      Vec3 p0_to_p1 = pos[1] - pos[0];
      Vec3 p0_to_p2 = pos[2] - pos[0];
      Vec3 normal = Cross(p0_to_p1, p0_to_p2);
      if (Dot(normal, p0_to_camera) > 0) {
        for (int j = 0; j < 3; j++) {
          //@Note: Camera
          pos[j] = pos[j] - camera_pos;
          //@Note: Perspective
          pos[j].x /= pos[j].z;
          pos[j].y /= pos[j].z;
          //@Note: To pixel space
          pos[j].x *= screen.x / 2;
          pos[j].y *= screen.y / 2;
          pos[j].x += screen.x / 2;
          pos[j].y += screen.y / 2;
        }
        DrawTriangle(&screen, &img, pos[0], pos[1], pos[2], face->tex[0], face->tex[1], face->tex[2]);
        DrawLine(&screen, pos[0].x, pos[0].y, pos[1].x, pos[1].y);
        DrawLine(&screen, pos[1].x, pos[1].y, pos[2].x, pos[2].y);
        DrawLine(&screen, pos[2].x, pos[2].y, pos[0].x, pos[0].y);
      }
    }
#endif
  }
}
