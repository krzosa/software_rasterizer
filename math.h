#include "main.h"
#include <math.h>
#include <stdint.h>

struct Mat4 {
  float p[4][4];
};

struct Vec2 {
  float x, y;
};

union Vec3 {
  struct { float x, y, z; };
  struct { float r, g, b; };
};

union Vec4 {
  struct { float x, y, z, w; };
  struct { float r, g, b, a; };
  struct { Vec3 xyz; };
};

FUNCTION
Mat4 Mat4Identity() {
  Mat4 result = {
    1,0,0,0,
    0,1,0,0,
    0,0,1,0,
    0,0,0,1,
  };
  return result;
}

FUNCTION
Mat4 Mat4RotationZ(float rotation) {
  float s = sinf(rotation);
  float c = cosf(rotation);
  Mat4 result = {
    c, s, 0, 0,
    -s,  c, 0, 0,
    0,  0, 1, 0,
    0,  0, 0, 1,
  };
  return result;
}

FUNCTION
Mat4 Mat4RotationY(float rotation) {
  float s = sinf(rotation);
  float c = cosf(rotation);
  Mat4 result = {
    c, 0, -s, 0,
    0, 1,  0, 0,
    s, 0,  c, 0,
    0, 0,  0, 1,
  };
  return result;
}

FUNCTION
Mat4 Mat4RotationX(float rotation) {
  float s = sinf(rotation);
  float c = cosf(rotation);
  Mat4 result = {
    1,  0, 0, 0,
    0,  c, s, 0,
    0,  -s, c, 0,
    0,  0, 0, 1,
  };
  return result;
}

FUNCTION
Mat4 Mat4Translate(Mat4 a, Vec3 translation) {
  a.p[0][0] += translation.x;
  a.p[0][1] += translation.y;
  a.p[0][2] += translation.z;
  return a;
}

FUNCTION
Vec4 operator-(Vec4 a, Vec4 b) {
  Vec4 result = {
    a.x - b.x,
    a.y - b.y,
    a.z - b.z,
    a.w - b.w
  };
  return result;
}

FUNCTION
Vec4 operator+(Vec4 a, Vec4 b) {
  Vec4 result = {
    a.x + b.x,
    a.y + b.y,
    a.z + b.z,
    a.w + b.w
  };
  return result;
}

FUNCTION
Vec3 operator-(Vec3 a, Vec3 b) {
  Vec3 result = {
    a.x - b.x,
    a.y - b.y,
    a.z - b.z,
  };
  return result;
}

FUNCTION
Vec3 operator+(Vec3 a, Vec3 b) {
  Vec3 result = {
    a.x + b.x,
    a.y + b.y,
    a.z + b.z,
  };
  return result;
}


FUNCTION
Vec4 operator*(Mat4 a, Vec4 b) {
  Vec4 result = {
    a.p[0][0] * b.x + a.p[0][1] * b.y + a.p[0][2] * b.z + a.p[0][3] * b.w,
    a.p[1][0] * b.x + a.p[1][1] * b.y + a.p[1][2] * b.z + a.p[1][3] * b.w,
    a.p[2][0] * b.x + a.p[2][1] * b.y + a.p[2][2] * b.z + a.p[2][3] * b.w,
    a.p[3][0] * b.x + a.p[3][1] * b.y + a.p[3][2] * b.z + a.p[3][3] * b.w,
  };
  return result;
}

FUNCTION
Vec3 operator*(Mat4 a, Vec3 b) {
  Vec4 result= {
    a.p[0][0] * b.x + a.p[0][1] * b.y + a.p[0][2] * b.z + a.p[0][3] * 1,
    a.p[1][0] * b.x + a.p[1][1] * b.y + a.p[1][2] * b.z + a.p[1][3] * 1,
    a.p[2][0] * b.x + a.p[2][1] * b.y + a.p[2][2] * b.z + a.p[2][3] * 1,
    a.p[3][0] * b.x + a.p[3][1] * b.y + a.p[3][2] * b.z + a.p[3][3] * 1,
  };
  ASSERT(result.w == 1);
  return result.xyz;
}

FUNCTION
Mat4 operator*(Mat4 a, Mat4 b) {
  Mat4 result;
  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 4; x++) {
      result.p[y][x] = a.p[y][0] * b.p[0][x] + a.p[y][1] * b.p[1][x] + a.p[y][2] * b.p[2][x] + a.p[y][3] * b.p[3][x];
    }
  }
  return result;
}

FUNCTION
float Dot(Vec3 a, Vec3 b) {
  float result = a.x * b.x + a.y * b.y + a.z * b.z;
  return result;
}

FUNCTION
Vec3 Cross(Vec3 a, Vec3 b) {
  Vec3 result = {
    a.y * b.z - a.z * b.y,
    a.z * b.x - a.x * b.z,
    a.x * b.y - a.y * b.x,
  };
  return result;
}

FUNCTION
uint32_t ColorToU32ARGB(Vec4 a) {
  uint8_t r8 = (uint8_t)(a.r * 255.f + 0.5f);
  uint8_t g8 = (uint8_t)(a.g * 255.f + 0.5f);
  uint8_t b8 = (uint8_t)(a.b * 255.f + 0.5f);
  uint8_t a8 = (uint8_t)(a.a * 255.f + 0.5f);
  uint32_t result = a8 << 24 | r8 << 16 | g8 << 8 | b8;
  return result;
}
