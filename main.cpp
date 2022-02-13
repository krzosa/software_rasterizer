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
};

GLOBAL Vec3 cube_vertices[] = {
  {-1,1,1}, {1,1,1}, {1,-1,1}, {-1,-1,1},
  {-1,1,-1}, {1,1,-1}, {1,-1,-1}, {-1,-1,-1},
};

GLOBAL Face cube_faces[] = {
  {4,1,2}, {4,2,3},
  {8,4,3}, {8,3,7},
  {8,5,1}, {8,1,4},
  {3,2,6}, {3,6,7},
  {7,6,5}, {7,5,8},
  {1,5,6}, {1,6,2}
};

FUNCTION
void DrawRect(Image* dst, float X, float Y, float w, float h) {
  int max_x = (int)(MIN(X + w, dst->x) + 0.5f);
  int max_y = (int)(MIN(Y + h, dst->y) + 0.5f);
  int min_x = (int)(MAX(0, X) + 0.5f);
  int min_y = (int)(MAX(0, Y) + 0.5f);

  for (int y = min_y; y < max_y; y++) {
    for (int x = min_x; x < max_x; x++) {
      dst->pixels[x + y * dst->x] = 0xffff0000;
    }
  }
}

FUNCTION
float EdgeFunction(Vec3 vecp0, Vec3 vecp1, Vec3 p) {
  float result = (vecp1.y - vecp0.y) * (p.x - vecp0.x) - (vecp1.x - vecp0.x) * (p.y - vecp0.y);
  return result;
}

FUNCTION 
void DrawTriangle(Image* dst, Vec3 p0, Vec3 p1, Vec3 p2) {
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

      if (edge1 <= 0 && edge2 <= 0 && edge3 <= 0) {
        int xi = (int)(x + 0.5f);
        int yi = (int)(y + 0.5f);
        float w1 = edge1 / area;
        float w2 = edge2 / area;
        float w3 = edge3 / area;
        float r = 1 * w1 + 0 * w2 + 0 * w3;
        float g = 0 * w1 + 1 * w2 + 0 * w3;
        float b = 0 * w1 + 0 * w2 + 1 * w3;
        uint8_t r8 = (uint8_t)((r * 255.f) + 0.5f);
        uint8_t g8 = (uint8_t)((g * 255.f) + 0.5f);
        uint8_t b8 = (uint8_t)((b * 255.f) + 0.5f);
        dst->pixels[xi + yi * dst->x] = r8 << 16 | g8 << 8 | b8;
      }
    }
  }
  DrawRect(dst, p0.x-4, p0.y-4, 8,8);
  DrawRect(dst, p1.x-4, p1.y-4, 8,8);
  DrawRect(dst, p2.x-4, p2.y-4, 8,8);
}

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
    dst->pixels[x + y * dst->x] = 0xffffffff;
    current_x += x_inc;
    current_y += y_inc;
  }
}


int main() {
  OS_Init({.window_x=1280, .window_y=720});
  float rotation = 0;
  Vec3 camera_pos = {0,0,-5};
  while (OS_GameLoop()) {
    for (int y = 0; y < screen.y; y++) {
      for (int x = 0; x < screen.x; x++) {
        screen.pixels[x + y * screen.x] = 0;
      }
    }
    
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
      if (Dot(normal, p0_to_camera) < 0) {
        for (int j = 0; j < 3; j++) {
          //@Note: Camera
          pos[j].x -= camera_pos.x;
          pos[j].y -= camera_pos.y;
          pos[j].z -= camera_pos.z;
          //@Note: Perspective
          pos[j].x /= pos[j].z;
          pos[j].y /= pos[j].z;
          //@Note: To pixel space
          pos[j].x *= screen.x / 2;
          pos[j].y *= screen.y / 2;
          pos[j].x += screen.x / 2;
          pos[j].y += screen.y / 2;
        }
        DrawTriangle(&screen, pos[0], pos[1], pos[2]);
        DrawLine(&screen, pos[0].x, pos[0].y, pos[1].x, pos[1].y);
        DrawLine(&screen, pos[1].x, pos[1].y, pos[2].x, pos[2].y);
        DrawLine(&screen, pos[2].x, pos[2].y, pos[0].x, pos[0].y);
      }
    }
  }
}
