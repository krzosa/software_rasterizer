#include "main.h"
#include <intrin.h>

constexpr float PI32 = 3.14159265359f;

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
Vec4 vec4(Vec3 a, float b) {
  Vec4 result = { a.x,a.y,a.z,b };
  return result;
}

FUNCTION
Mat4 make_matrix_identity() {
  Mat4 result = {
    1,0,0,0,
    0,1,0,0,
    0,0,1,0,
    0,0,0,1,
  };
  return result;
}

FUNCTION
float sin(float value) {
  __m128 result128 = _mm_set_ps1(value);
  result128 = _mm_sin_ps(result128);
  float result = *(float *)&result128;
  return result;
}

FUNCTION
float cos(float value) {
  __m128 result128 = _mm_set_ps1(value);
  result128 = _mm_cos_ps(result128);
  float result = *(float*)&result128;
  return result;
}

FUNCTION
float tan(float value) {
  __m128 result128 = _mm_set_ps1(value);
  result128 = _mm_tan_ps(result128);
  float result = *(float*)&result128;
  return result;
}

FUNCTION
float floor(float value) {
  __m128 result128 = _mm_set_ps1(value);
  result128 = _mm_floor_ps(result128);
  float result = *(float*)&result128;
  return result;
}

FUNCTION
float ceil(float value) {
  __m128 result128 = _mm_set_ps1(value);
  result128 = _mm_ceil_ps(result128);
  float result = *(float*)&result128;
  return result;
}

FUNCTION
float round(float value) {
  __m128 result128 = _mm_set_ps1(value);
  result128 = _mm_round_ps(result128, _MM_FROUND_TO_NEAREST_INT| _MM_FROUND_NO_EXC);
  float result = *(float*)&result128;
  return result;
}

FUNCTION
float sqrt(float value) {
  __m128 result128 = _mm_set_ps1(value);
  result128 = _mm_sqrt_ps(result128);
  float result = *(float*)&result128;
  return result;
}

FUNCTION
Mat4 make_matrix_rotation_x(float rotation) {
  float s = sin(rotation);
  float c = cos(rotation);
  Mat4 result = {
    c, s, 0, 0,
    -s,  c, 0, 0,
    0,  0, 1, 0,
    0,  0, 0, 1,
  };
  return result;
}

FUNCTION
Mat4 make_matrix_rotation_y(float rotation) {
  float s = sin(rotation);
  float c = cos(rotation);
  Mat4 result = {
    c, 0, -s, 0,
    0, 1,  0, 0,
    s, 0,  c, 0,
    0, 0,  0, 1,
  };
  return result;
}

FUNCTION
Mat4 make_matrix_rotation_z(float rotation) {
  float s = sin(rotation);
  float c = cos(rotation);
  Mat4 result = {
    1,  0, 0, 0,
    0,  c, s, 0,
    0,  -s, c, 0,
    0,  0, 0, 1,
  };
  return result;
}

FUNCTION
Mat4 make_matrix_perspective(float fov, float window_x, float window_y, float znear, float zfar) {
  float aspect_ratio = window_y / window_x;
  float f = (1.f / tan((fov/2.f)*(180.f/PI32)));
  Mat4 result = {
    aspect_ratio*f, 0, 0, 0,
    0, f, 0, 0,
    0, 0, (zfar)-(zfar-znear),(-zfar*znear)-(zfar - znear),
    0,0,1,0
  };
  return result;
}

FUNCTION
Mat4 transpose(Mat4 a) {
  Mat4 result = a;
  result.p[0][1] = result.p[1][0];
  result.p[0][2] = result.p[2][0];
  result.p[0][3] = result.p[3][0];
  result.p[2][1] = result.p[1][2];
  result.p[3][1] = result.p[1][3];
  result.p[3][2] = result.p[2][3];
  return result;
}

FUNCTION
Mat4 translate(Mat4 a, Vec3 translation) {
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
float dot(Vec3 a, Vec3 b) {
  float result = a.x * b.x + a.y * b.y + a.z * b.z;
  return result;
}

FUNCTION
Vec3 cross(Vec3 a, Vec3 b) {
  Vec3 result = {
    a.y * b.z - a.z * b.y,
    a.z * b.x - a.x * b.z,
    a.x * b.y - a.y * b.x,
  };
  return result;
}

FUNCTION
U32 color_to_u32argb(Vec4 a) {
  uint8_t r8 = (uint8_t)(a.r * 255.f);
  uint8_t g8 = (uint8_t)(a.g * 255.f);
  uint8_t b8 = (uint8_t)(a.b * 255.f);
  uint8_t a8 = (uint8_t)(a.a * 255.f);
  U32 result = a8 << 24 | r8 << 16 | g8 << 8 | b8;
  return result;
}

FUNCTION
U32 color_to_u32abgr(Vec4 a) {
  uint8_t r8 = (uint8_t)(a.r * 255.f);
  uint8_t g8 = (uint8_t)(a.g * 255.f);
  uint8_t b8 = (uint8_t)(a.b * 255.f);
  uint8_t a8 = (uint8_t)(a.a * 255.f);
  U32 result = a8 << 24 | b8 << 16 | g8 << 8 | r8;
  return result;
}

FUNCTION
Vec4 v4argb(U32 c) {
  float a = ((c & 0xff000000) >> 24) / 255.f;
  float r = ((c & 0x00ff0000) >> 16) / 255.f;
  float g = ((c & 0x0000ff00) >> 8) / 255.f;
  float b = ((c & 0x000000ff) >> 0) / 255.f;
  Vec4 result = { r,g,b,a };
  return result;
}

FUNCTION
Vec4 vec4abgr(U32 c) {
  float a = ((c & 0xff000000) >> 24) / 255.f;
  float b = ((c & 0x00ff0000) >> 16) / 255.f;
  float g = ((c & 0x0000ff00) >> 8) / 255.f;
  float r = ((c & 0x000000ff) >> 0) / 255.f;
  Vec4 result = { r,g,b,a };
  return result;
}

FUNCTION
float lerp(float a, float b, float t) {
  float result = a + (b-a)*t; //(1.0f - t) * a + t * b;
  return result;
}

FUNCTION
Vec4 lerp(Vec4 a, Vec4 b, float t) {
  Vec4 result = {lerp(a.x,b.x,t), lerp(a.y,b.y,t), lerp(a.z,b.z,t), lerp(a.w,b.w,t) };
  return result;
}