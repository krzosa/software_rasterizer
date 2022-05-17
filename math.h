#pragma once

///////////////////////////////////////
// @Section Math
#include <math.h>
constexpr F32 PI32 = 3.14159265359f;

api F32 power(S64 pow, F32 value) {
  F32 result = value;
  if (pow == 0) {
    result = 1;
  } else {
    for (S64 i = 1; i < pow; i++) {
      result *= result;
    }
  }
  return result;
}

api F32 to_radians(F32 degrees) {
  F32 result = degrees * (PI32 / 180.f);
  return result;
}

api F32 to_degrees(F32 radians) {
  F32 result = radians * (180.f / PI32);
  return result;
}

api F32 fraction(F32 value) {
  F32 result = value - floorf(value);
  return result;
}

api F32 absolute(F32 value) {
  if (value < 0)
    value = -value;
  return value;
}

api S32 square(S32 val) { return val * val; }

api S32 clamp01(S32 val) {
  S32 result = clamp(0, val, 1);
  return result;
}

api S32 sign(S32 val) {
  if (val > 0)
    return 1;
  else if (val < 0)
    return -1;
  return 0;
}

api F32 square(F32 val) { return val * val; }

api F32 clamp01(F32 val) {
  F32 result = clamp(0.f, val, 1.f);
  return result;
}

api F32 sign(F32 val) {
  if (val > 0)
    return 1;
  else if (val < 0)
    return -1;
  return 0;
}

api F32 floor(F32 val) { return floorf(val); }

api F32 ceil(F32 val) { return ceilf(val); }

api F32 round(F32 val) { return roundf(val); }

api F32 sine(F32 val) { return sinf(val); }

api F32 sine01(F32 val) {
  F32 result = sine(val);
  result += 1;
  result /= 2;
  return result;
}

api F32 cosine(F32 val) { return cosf(val); }

api F32 square_root(F32 val) { return sqrtf(val); }

api Vec2 vec2(F32 x, F32 y) {
  Vec2 result;
  result.x = x;
  result.y = y;
  return result;
}
api Vec2 vec2() { return vec2(0, 0); }
api Vec2 operator+(Vec2 a, Vec2 b) {
  Vec2 result = vec2(a.x + b.x, a.y + b.y);
  return result;
}
api Vec2 operator+(Vec2 a, F32 b) {
  Vec2 result = vec2(a.x + b, a.y + b);
  return result;
}
api Vec2 operator+(F32 a, Vec2 b) {
  Vec2 result = vec2(a + b.x, a + b.y);
  return result;
}
api Vec2 &operator+=(Vec2 &a, Vec2 b) {
  a = a + b;
  return a;
}
api Vec2 &operator+=(Vec2 &a, F32 b) {
  a = a + b;
  return a;
}
api Vec2 operator-(Vec2 a, Vec2 b) {
  Vec2 result = vec2(a.x - b.x, a.y - b.y);
  return result;
}
api Vec2 operator-(Vec2 a, F32 b) {
  Vec2 result = vec2(a.x - b, a.y - b);
  return result;
}
api Vec2 operator-(F32 a, Vec2 b) {
  Vec2 result = vec2(a - b.x, a - b.y);
  return result;
}
api Vec2 &operator-=(Vec2 &a, Vec2 b) {
  a = a - b;
  return a;
}
api Vec2 &operator-=(Vec2 &a, F32 b) {
  a = a - b;
  return a;
}
api Vec2 operator*(Vec2 a, Vec2 b) {
  Vec2 result = vec2(a.x * b.x, a.y * b.y);
  return result;
}
api Vec2 operator*(Vec2 a, F32 b) {
  Vec2 result = vec2(a.x * b, a.y * b);
  return result;
}
api Vec2 operator*(F32 a, Vec2 b) {
  Vec2 result = vec2(a * b.x, a * b.y);
  return result;
}
api Vec2 &operator*=(Vec2 &a, Vec2 b) {
  a = a * b;
  return a;
}
api Vec2 &operator*=(Vec2 &a, F32 b) {
  a = a * b;
  return a;
}
api Vec2 operator/(Vec2 a, Vec2 b) {
  Vec2 result = vec2(a.x / b.x, a.y / b.y);
  return result;
}
api Vec2 operator/(Vec2 a, F32 b) {
  Vec2 result = vec2(a.x / b, a.y / b);
  return result;
}
api Vec2 operator/(F32 a, Vec2 b) {
  Vec2 result = vec2(a / b.x, a / b.y);
  return result;
}
api Vec2 &operator/=(Vec2 &a, Vec2 b) {
  a = a / b;
  return a;
}
api Vec2 &operator/=(Vec2 &a, F32 b) {
  a = a / b;
  return a;
}
api B32 operator!=(Vec2 a, Vec2 b) {
  B32 result = (a.x != b.x) || (a.y != b.y);
  return result;
}
api B32 operator==(Vec2 a, Vec2 b) {
  B32 result = (a.x == b.x) && (a.y == b.y);
  return result;
}
api Vec2 operator-(Vec2 a) {
  Vec2 result = vec2(-a.x, -a.y);
  return result;
}
api Vec2 clamp(Vec2 min, Vec2 val, Vec2 max) {
  Vec2 result = vec2(clamp(min.x, val.x, max.x), clamp(min.y, val.y, max.y));
  return result;
}
api Vec2 clamp(F32 min, Vec2 val, F32 max) {
  Vec2 result = vec2(clamp(min, val.x, max), clamp(min, val.y, max));
  return result;
}
api Vec2 clamp01(Vec2 val) {
  Vec2 result = vec2(clamp01(val.x), clamp01(val.y));
  return result;
}
api Vec2 ceil(Vec2 a) {
  Vec2 result = vec2(ceil(a.x), ceil(a.y));
  return result;
}
api Vec2 floor(Vec2 a) {
  Vec2 result = vec2(floor(a.x), floor(a.y));
  return result;
}
api Vec2 round(Vec2 a) {
  Vec2 result = vec2(round(a.x), round(a.y));
  return result;
}
api Vec2 absolute(Vec2 a) {
  Vec2 result = vec2(absolute(a.x), absolute(a.y));
  return result;
}
api Vec2 sign(Vec2 a) {
  Vec2 result = vec2(sign(a.x), sign(a.y));
  return result;
}
api Vec2 fraction(Vec2 a) {
  Vec2 result = vec2(fraction(a.x), fraction(a.y));
  return result;
}
api Vec2 square(Vec2 a) {
  Vec2 result = vec2(square(a.x), square(a.y));
  return result;
}
api Vec2 square_root(Vec2 a) {
  Vec2 result = vec2(square_root(a.x), square_root(a.y));
  return result;
}
api F32 dot(Vec2 a, Vec2 b) {
  F32 result = a.x * b.x + a.y * b.y;
  return result;
}
api F32 length_squared(Vec2 a) {
  F32 result = dot(a, a);
  return result;
}
api F32 length(Vec2 a) {
  F32 result = square_root(length_squared(a));
  return result;
}
api Vec2 lerp(Vec2 from, Vec2 to, F32 t) {
  Vec2 result = (1 - t) * from + to * t;
  return result;
}
api Vec3 vec3(F32 x, F32 y, F32 z) {
  Vec3 result;
  result.x = x;
  result.y = y;
  result.z = z;
  return result;
}
api Vec3 vec3() { return vec3(0, 0, 0); }
api Vec3 operator+(Vec3 a, Vec3 b) {
  Vec3 result = vec3(a.x + b.x, a.y + b.y, a.z + b.z);
  return result;
}
api Vec3 operator+(Vec3 a, F32 b) {
  Vec3 result = vec3(a.x + b, a.y + b, a.z + b);
  return result;
}
api Vec3 operator+(F32 a, Vec3 b) {
  Vec3 result = vec3(a + b.x, a + b.y, a + b.z);
  return result;
}
api Vec3 &operator+=(Vec3 &a, Vec3 b) {
  a = a + b;
  return a;
}
api Vec3 &operator+=(Vec3 &a, F32 b) {
  a = a + b;
  return a;
}
api Vec3 operator-(Vec3 a, Vec3 b) {
  Vec3 result = vec3(a.x - b.x, a.y - b.y, a.z - b.z);
  return result;
}
api Vec3 operator-(Vec3 a, F32 b) {
  Vec3 result = vec3(a.x - b, a.y - b, a.z - b);
  return result;
}
api Vec3 operator-(F32 a, Vec3 b) {
  Vec3 result = vec3(a - b.x, a - b.y, a - b.z);
  return result;
}
api Vec3 &operator-=(Vec3 &a, Vec3 b) {
  a = a - b;
  return a;
}
api Vec3 &operator-=(Vec3 &a, F32 b) {
  a = a - b;
  return a;
}
api Vec3 operator*(Vec3 a, Vec3 b) {
  Vec3 result = vec3(a.x * b.x, a.y * b.y, a.z * b.z);
  return result;
}
api Vec3 operator*(Vec3 a, F32 b) {
  Vec3 result = vec3(a.x * b, a.y * b, a.z * b);
  return result;
}
api Vec3 operator*(F32 a, Vec3 b) {
  Vec3 result = vec3(a * b.x, a * b.y, a * b.z);
  return result;
}
api Vec3 &operator*=(Vec3 &a, Vec3 b) {
  a = a * b;
  return a;
}
api Vec3 &operator*=(Vec3 &a, F32 b) {
  a = a * b;
  return a;
}
api Vec3 operator/(Vec3 a, Vec3 b) {
  Vec3 result = vec3(a.x / b.x, a.y / b.y, a.z / b.z);
  return result;
}
api Vec3 operator/(Vec3 a, F32 b) {
  Vec3 result = vec3(a.x / b, a.y / b, a.z / b);
  return result;
}
api Vec3 operator/(F32 a, Vec3 b) {
  Vec3 result = vec3(a / b.x, a / b.y, a / b.z);
  return result;
}
api Vec3 &operator/=(Vec3 &a, Vec3 b) {
  a = a / b;
  return a;
}
api Vec3 &operator/=(Vec3 &a, F32 b) {
  a = a / b;
  return a;
}
api B32 operator!=(Vec3 a, Vec3 b) {
  B32 result = (a.x != b.x) || (a.y != b.y) || (a.z != b.z);
  return result;
}
api B32 operator==(Vec3 a, Vec3 b) {
  B32 result = (a.x == b.x) && (a.y == b.y) && (a.z == b.z);
  return result;
}
api Vec3 operator-(Vec3 a) {
  Vec3 result = vec3(-a.x, -a.y, -a.z);
  return result;
}
api Vec3 clamp(Vec3 min, Vec3 val, Vec3 max) {
  Vec3 result = vec3(clamp(min.x, val.x, max.x), clamp(min.y, val.y, max.y), clamp(min.z, val.z, max.z));
  return result;
}
api Vec3 clamp(F32 min, Vec3 val, F32 max) {
  Vec3 result = vec3(clamp(min, val.x, max), clamp(min, val.y, max), clamp(min, val.z, max));
  return result;
}
api Vec3 clamp01(Vec3 val) {
  Vec3 result = vec3(clamp01(val.x), clamp01(val.y), clamp01(val.z));
  return result;
}
api Vec3 ceil(Vec3 a) {
  Vec3 result = vec3(ceil(a.x), ceil(a.y), ceil(a.z));
  return result;
}
api Vec3 floor(Vec3 a) {
  Vec3 result = vec3(floor(a.x), floor(a.y), floor(a.z));
  return result;
}
api Vec3 round(Vec3 a) {
  Vec3 result = vec3(round(a.x), round(a.y), round(a.z));
  return result;
}
api Vec3 absolute(Vec3 a) {
  Vec3 result = vec3(absolute(a.x), absolute(a.y), absolute(a.z));
  return result;
}
api Vec3 sign(Vec3 a) {
  Vec3 result = vec3(sign(a.x), sign(a.y), sign(a.z));
  return result;
}
api Vec3 fraction(Vec3 a) {
  Vec3 result = vec3(fraction(a.x), fraction(a.y), fraction(a.z));
  return result;
}
api Vec3 square(Vec3 a) {
  Vec3 result = vec3(square(a.x), square(a.y), square(a.z));
  return result;
}
api Vec3 square_root(Vec3 a) {
  Vec3 result = vec3(square_root(a.x), square_root(a.y), square_root(a.z));
  return result;
}
api F32 dot(Vec3 a, Vec3 b) {
  F32 result = a.x * b.x + a.y * b.y + a.z * b.z;
  return result;
}
api F32 length_squared(Vec3 a) {
  F32 result = dot(a, a);
  return result;
}
api F32 length(Vec3 a) {
  F32 result = square_root(length_squared(a));
  return result;
}
api Vec3 lerp(Vec3 from, Vec3 to, F32 t) {
  Vec3 result = (1 - t) * from + to * t;
  return result;
}
api Vec4 vec4(F32 x, F32 y, F32 z, F32 w) {
  Vec4 result;
  result.x = x;
  result.y = y;
  result.z = z;
  result.w = w;
  return result;
}
api Vec4 vec4() { return vec4(0, 0, 0, 0); }
api Vec4 operator+(Vec4 a, Vec4 b) {
  Vec4 result = vec4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
  return result;
}
api Vec4 operator+(Vec4 a, F32 b) {
  Vec4 result = vec4(a.x + b, a.y + b, a.z + b, a.w + b);
  return result;
}
api Vec4 operator+(F32 a, Vec4 b) {
  Vec4 result = vec4(a + b.x, a + b.y, a + b.z, a + b.w);
  return result;
}
api Vec4 &operator+=(Vec4 &a, Vec4 b) {
  a = a + b;
  return a;
}
api Vec4 &operator+=(Vec4 &a, F32 b) {
  a = a + b;
  return a;
}
api Vec4 operator-(Vec4 a, Vec4 b) {
  Vec4 result = vec4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
  return result;
}
api Vec4 operator-(Vec4 a, F32 b) {
  Vec4 result = vec4(a.x - b, a.y - b, a.z - b, a.w - b);
  return result;
}
api Vec4 operator-(F32 a, Vec4 b) {
  Vec4 result = vec4(a - b.x, a - b.y, a - b.z, a - b.w);
  return result;
}
api Vec4 &operator-=(Vec4 &a, Vec4 b) {
  a = a - b;
  return a;
}
api Vec4 &operator-=(Vec4 &a, F32 b) {
  a = a - b;
  return a;
}
api Vec4 operator*(Vec4 a, Vec4 b) {
  Vec4 result = vec4(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
  return result;
}
api Vec4 operator*(Vec4 a, F32 b) {
  Vec4 result = vec4(a.x * b, a.y * b, a.z * b, a.w * b);
  return result;
}
api Vec4 operator*(F32 a, Vec4 b) {
  Vec4 result = vec4(a * b.x, a * b.y, a * b.z, a * b.w);
  return result;
}
api Vec4 &operator*=(Vec4 &a, Vec4 b) {
  a = a * b;
  return a;
}
api Vec4 &operator*=(Vec4 &a, F32 b) {
  a = a * b;
  return a;
}
api Vec4 operator/(Vec4 a, Vec4 b) {
  Vec4 result = vec4(a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w);
  return result;
}
api Vec4 operator/(Vec4 a, F32 b) {
  Vec4 result = vec4(a.x / b, a.y / b, a.z / b, a.w / b);
  return result;
}
api Vec4 operator/(F32 a, Vec4 b) {
  Vec4 result = vec4(a / b.x, a / b.y, a / b.z, a / b.w);
  return result;
}
api Vec4 &operator/=(Vec4 &a, Vec4 b) {
  a = a / b;
  return a;
}
api Vec4 &operator/=(Vec4 &a, F32 b) {
  a = a / b;
  return a;
}
api B32 operator!=(Vec4 a, Vec4 b) {
  B32 result = (a.x != b.x) || (a.y != b.y) || (a.z != b.z) || (a.w != b.w);
  return result;
}
api B32 operator==(Vec4 a, Vec4 b) {
  B32 result = (a.x == b.x) && (a.y == b.y) && (a.z == b.z) && (a.w == b.w);
  return result;
}
api Vec4 operator-(Vec4 a) {
  Vec4 result = vec4(-a.x, -a.y, -a.z, -a.w);
  return result;
}
api Vec4 clamp(Vec4 min, Vec4 val, Vec4 max) {
  Vec4 result = vec4(clamp(min.x, val.x, max.x), clamp(min.y, val.y, max.y), clamp(min.z, val.z, max.z),
                     clamp(min.w, val.w, max.w));
  return result;
}
api Vec4 clamp(F32 min, Vec4 val, F32 max) {
  Vec4 result =
    vec4(clamp(min, val.x, max), clamp(min, val.y, max), clamp(min, val.z, max), clamp(min, val.w, max));
  return result;
}
api Vec4 clamp01(Vec4 val) {
  Vec4 result = vec4(clamp01(val.x), clamp01(val.y), clamp01(val.z), clamp01(val.w));
  return result;
}
api Vec4 ceil(Vec4 a) {
  Vec4 result = vec4(ceil(a.x), ceil(a.y), ceil(a.z), ceil(a.w));
  return result;
}
api Vec4 floor(Vec4 a) {
  Vec4 result = vec4(floor(a.x), floor(a.y), floor(a.z), floor(a.w));
  return result;
}
api Vec4 round(Vec4 a) {
  Vec4 result = vec4(round(a.x), round(a.y), round(a.z), round(a.w));
  return result;
}
api Vec4 absolute(Vec4 a) {
  Vec4 result = vec4(absolute(a.x), absolute(a.y), absolute(a.z), absolute(a.w));
  return result;
}
api Vec4 sign(Vec4 a) {
  Vec4 result = vec4(sign(a.x), sign(a.y), sign(a.z), sign(a.w));
  return result;
}
api Vec4 fraction(Vec4 a) {
  Vec4 result = vec4(fraction(a.x), fraction(a.y), fraction(a.z), fraction(a.w));
  return result;
}
api Vec4 square(Vec4 a) {
  Vec4 result = vec4(square(a.x), square(a.y), square(a.z), square(a.w));
  return result;
}
api Vec4 square_root(Vec4 a) {
  Vec4 result = vec4(square_root(a.x), square_root(a.y), square_root(a.z), square_root(a.w));
  return result;
}
api F32 dot(Vec4 a, Vec4 b) {
  F32 result = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
  return result;
}
api F32 length_squared(Vec4 a) {
  F32 result = dot(a, a);
  return result;
}
api F32 length(Vec4 a) {
  F32 result = square_root(length_squared(a));
  return result;
}
api Vec4 lerp(Vec4 from, Vec4 to, F32 t) {
  Vec4 result = (1 - t) * from + to * t;
  return result;
}
api Vec2I vec2i(S32 x, S32 y) {
  Vec2I result;
  result.x = x;
  result.y = y;
  return result;
}
api Vec2I vec2i() { return vec2i(0, 0); }
api Vec2I operator+(Vec2I a, Vec2I b) {
  Vec2I result = vec2i(a.x + b.x, a.y + b.y);
  return result;
}
api Vec2I operator+(Vec2I a, S32 b) {
  Vec2I result = vec2i(a.x + b, a.y + b);
  return result;
}
api Vec2I operator+(S32 a, Vec2I b) {
  Vec2I result = vec2i(a + b.x, a + b.y);
  return result;
}
api Vec2I &operator+=(Vec2I &a, Vec2I b) {
  a = a + b;
  return a;
}
api Vec2I &operator+=(Vec2I &a, S32 b) {
  a = a + b;
  return a;
}
api Vec2I operator-(Vec2I a, Vec2I b) {
  Vec2I result = vec2i(a.x - b.x, a.y - b.y);
  return result;
}
api Vec2I operator-(Vec2I a, S32 b) {
  Vec2I result = vec2i(a.x - b, a.y - b);
  return result;
}
api Vec2I operator-(S32 a, Vec2I b) {
  Vec2I result = vec2i(a - b.x, a - b.y);
  return result;
}
api Vec2I &operator-=(Vec2I &a, Vec2I b) {
  a = a - b;
  return a;
}
api Vec2I &operator-=(Vec2I &a, S32 b) {
  a = a - b;
  return a;
}
api Vec2I operator*(Vec2I a, Vec2I b) {
  Vec2I result = vec2i(a.x * b.x, a.y * b.y);
  return result;
}
api Vec2I operator*(Vec2I a, S32 b) {
  Vec2I result = vec2i(a.x * b, a.y * b);
  return result;
}
api Vec2I operator*(S32 a, Vec2I b) {
  Vec2I result = vec2i(a * b.x, a * b.y);
  return result;
}
api Vec2I &operator*=(Vec2I &a, Vec2I b) {
  a = a * b;
  return a;
}
api Vec2I &operator*=(Vec2I &a, S32 b) {
  a = a * b;
  return a;
}
api Vec2I operator/(Vec2I a, Vec2I b) {
  Vec2I result = vec2i(a.x / b.x, a.y / b.y);
  return result;
}
api Vec2I operator/(Vec2I a, S32 b) {
  Vec2I result = vec2i(a.x / b, a.y / b);
  return result;
}
api Vec2I operator/(S32 a, Vec2I b) {
  Vec2I result = vec2i(a / b.x, a / b.y);
  return result;
}
api Vec2I &operator/=(Vec2I &a, Vec2I b) {
  a = a / b;
  return a;
}
api Vec2I &operator/=(Vec2I &a, S32 b) {
  a = a / b;
  return a;
}
api B32 operator!=(Vec2I a, Vec2I b) {
  B32 result = (a.x != b.x) || (a.y != b.y);
  return result;
}
api B32 operator==(Vec2I a, Vec2I b) {
  B32 result = (a.x == b.x) && (a.y == b.y);
  return result;
}
api Vec2I operator-(Vec2I a) {
  Vec2I result = vec2i(-a.x, -a.y);
  return result;
}
api Vec2I clamp(Vec2I min, Vec2I val, Vec2I max) {
  Vec2I result = vec2i(clamp(min.x, val.x, max.x), clamp(min.y, val.y, max.y));
  return result;
}
api Vec2I clamp(S32 min, Vec2I val, S32 max) {
  Vec2I result = vec2i(clamp(min, val.x, max), clamp(min, val.y, max));
  return result;
}
api Vec2I clamp01(Vec2I val) {
  Vec2I result = vec2i(clamp01(val.x), clamp01(val.y));
  return result;
}
api Vec3I vec3i(S32 x, S32 y, S32 z) {
  Vec3I result;
  result.x = x;
  result.y = y;
  result.z = z;
  return result;
}
api Vec3I vec3i() { return vec3i(0, 0, 0); }
api Vec3I operator+(Vec3I a, Vec3I b) {
  Vec3I result = vec3i(a.x + b.x, a.y + b.y, a.z + b.z);
  return result;
}
api Vec3I operator+(Vec3I a, S32 b) {
  Vec3I result = vec3i(a.x + b, a.y + b, a.z + b);
  return result;
}
api Vec3I operator+(S32 a, Vec3I b) {
  Vec3I result = vec3i(a + b.x, a + b.y, a + b.z);
  return result;
}
api Vec3I &operator+=(Vec3I &a, Vec3I b) {
  a = a + b;
  return a;
}
api Vec3I &operator+=(Vec3I &a, S32 b) {
  a = a + b;
  return a;
}
api Vec3I operator-(Vec3I a, Vec3I b) {
  Vec3I result = vec3i(a.x - b.x, a.y - b.y, a.z - b.z);
  return result;
}
api Vec3I operator-(Vec3I a, S32 b) {
  Vec3I result = vec3i(a.x - b, a.y - b, a.z - b);
  return result;
}
api Vec3I operator-(S32 a, Vec3I b) {
  Vec3I result = vec3i(a - b.x, a - b.y, a - b.z);
  return result;
}
api Vec3I &operator-=(Vec3I &a, Vec3I b) {
  a = a - b;
  return a;
}
api Vec3I &operator-=(Vec3I &a, S32 b) {
  a = a - b;
  return a;
}
api Vec3I operator*(Vec3I a, Vec3I b) {
  Vec3I result = vec3i(a.x * b.x, a.y * b.y, a.z * b.z);
  return result;
}
api Vec3I operator*(Vec3I a, S32 b) {
  Vec3I result = vec3i(a.x * b, a.y * b, a.z * b);
  return result;
}
api Vec3I operator*(S32 a, Vec3I b) {
  Vec3I result = vec3i(a * b.x, a * b.y, a * b.z);
  return result;
}
api Vec3I &operator*=(Vec3I &a, Vec3I b) {
  a = a * b;
  return a;
}
api Vec3I &operator*=(Vec3I &a, S32 b) {
  a = a * b;
  return a;
}
api Vec3I operator/(Vec3I a, Vec3I b) {
  Vec3I result = vec3i(a.x / b.x, a.y / b.y, a.z / b.z);
  return result;
}
api Vec3I operator/(Vec3I a, S32 b) {
  Vec3I result = vec3i(a.x / b, a.y / b, a.z / b);
  return result;
}
api Vec3I operator/(S32 a, Vec3I b) {
  Vec3I result = vec3i(a / b.x, a / b.y, a / b.z);
  return result;
}
api Vec3I &operator/=(Vec3I &a, Vec3I b) {
  a = a / b;
  return a;
}
api Vec3I &operator/=(Vec3I &a, S32 b) {
  a = a / b;
  return a;
}
api B32 operator!=(Vec3I a, Vec3I b) {
  B32 result = (a.x != b.x) || (a.y != b.y) || (a.z != b.z);
  return result;
}
api B32 operator==(Vec3I a, Vec3I b) {
  B32 result = (a.x == b.x) && (a.y == b.y) && (a.z == b.z);
  return result;
}
api Vec3I operator-(Vec3I a) {
  Vec3I result = vec3i(-a.x, -a.y, -a.z);
  return result;
}
api Vec3I clamp(Vec3I min, Vec3I val, Vec3I max) {
  Vec3I result = vec3i(clamp(min.x, val.x, max.x), clamp(min.y, val.y, max.y), clamp(min.z, val.z, max.z));
  return result;
}
api Vec3I clamp(S32 min, Vec3I val, S32 max) {
  Vec3I result = vec3i(clamp(min, val.x, max), clamp(min, val.y, max), clamp(min, val.z, max));
  return result;
}
api Vec3I clamp01(Vec3I val) {
  Vec3I result = vec3i(clamp01(val.x), clamp01(val.y), clamp01(val.z));
  return result;
}
api Vec4I v4i(S32 x, S32 y, S32 z, S32 w) {
  Vec4I result;
  result.x = x;
  result.y = y;
  result.z = z;
  result.w = w;
  return result;
}
api Vec4I v4i() { return v4i(0, 0, 0, 0); }
api Vec4I operator+(Vec4I a, Vec4I b) {
  Vec4I result = v4i(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
  return result;
}
api Vec4I operator+(Vec4I a, S32 b) {
  Vec4I result = v4i(a.x + b, a.y + b, a.z + b, a.w + b);
  return result;
}
api Vec4I operator+(S32 a, Vec4I b) {
  Vec4I result = v4i(a + b.x, a + b.y, a + b.z, a + b.w);
  return result;
}
api Vec4I &operator+=(Vec4I &a, Vec4I b) {
  a = a + b;
  return a;
}
api Vec4I &operator+=(Vec4I &a, S32 b) {
  a = a + b;
  return a;
}
api Vec4I operator-(Vec4I a, Vec4I b) {
  Vec4I result = v4i(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
  return result;
}
api Vec4I operator-(Vec4I a, S32 b) {
  Vec4I result = v4i(a.x - b, a.y - b, a.z - b, a.w - b);
  return result;
}
api Vec4I operator-(S32 a, Vec4I b) {
  Vec4I result = v4i(a - b.x, a - b.y, a - b.z, a - b.w);
  return result;
}
api Vec4I &operator-=(Vec4I &a, Vec4I b) {
  a = a - b;
  return a;
}
api Vec4I &operator-=(Vec4I &a, S32 b) {
  a = a - b;
  return a;
}
api Vec4I operator*(Vec4I a, Vec4I b) {
  Vec4I result = v4i(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
  return result;
}
api Vec4I operator*(Vec4I a, S32 b) {
  Vec4I result = v4i(a.x * b, a.y * b, a.z * b, a.w * b);
  return result;
}
api Vec4I operator*(S32 a, Vec4I b) {
  Vec4I result = v4i(a * b.x, a * b.y, a * b.z, a * b.w);
  return result;
}
api Vec4I &operator*=(Vec4I &a, Vec4I b) {
  a = a * b;
  return a;
}
api Vec4I &operator*=(Vec4I &a, S32 b) {
  a = a * b;
  return a;
}
api Vec4I operator/(Vec4I a, Vec4I b) {
  Vec4I result = v4i(a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w);
  return result;
}
api Vec4I operator/(Vec4I a, S32 b) {
  Vec4I result = v4i(a.x / b, a.y / b, a.z / b, a.w / b);
  return result;
}
api Vec4I operator/(S32 a, Vec4I b) {
  Vec4I result = v4i(a / b.x, a / b.y, a / b.z, a / b.w);
  return result;
}
api Vec4I &operator/=(Vec4I &a, Vec4I b) {
  a = a / b;
  return a;
}
api Vec4I &operator/=(Vec4I &a, S32 b) {
  a = a / b;
  return a;
}
api B32 operator!=(Vec4I a, Vec4I b) {
  B32 result = (a.x != b.x) || (a.y != b.y) || (a.z != b.z) || (a.w != b.w);
  return result;
}
api B32 operator==(Vec4I a, Vec4I b) {
  B32 result = (a.x == b.x) && (a.y == b.y) && (a.z == b.z) && (a.w == b.w);
  return result;
}
api Vec4I operator-(Vec4I a) {
  Vec4I result = v4i(-a.x, -a.y, -a.z, -a.w);
  return result;
}
api Vec4I clamp(Vec4I min, Vec4I val, Vec4I max) {
  Vec4I result = v4i(clamp(min.x, val.x, max.x), clamp(min.y, val.y, max.y), clamp(min.z, val.z, max.z),
                     clamp(min.w, val.w, max.w));
  return result;
}
api Vec4I clamp(S32 min, Vec4I val, S32 max) {
  Vec4I result =
    v4i(clamp(min, val.x, max), clamp(min, val.y, max), clamp(min, val.z, max), clamp(min, val.w, max));
  return result;
}
api Vec4I clamp01(Vec4I val) {
  Vec4I result = v4i(clamp01(val.x), clamp01(val.y), clamp01(val.z), clamp01(val.w));
  return result;
}
api Rect2 rect2(F32 min_x, F32 min_y, F32 max_x, F32 max_y) {
  Rect2 result;
  result.min_x = min_x;
  result.min_y = min_y;
  result.max_x = max_x;
  result.max_y = max_y;
  return result;
}
api Rect2 rect2() { return rect2(0, 0, 0, 0); }
api Rect2 operator+(Rect2 a, Rect2 b) {
  Rect2 result = rect2(a.min_x + b.min_x, a.min_y + b.min_y, a.max_x + b.max_x, a.max_y + b.max_y);
  return result;
}
api Rect2 operator+(Rect2 a, F32 b) {
  Rect2 result = rect2(a.min_x + b, a.min_y + b, a.max_x + b, a.max_y + b);
  return result;
}
api Rect2 operator+(F32 a, Rect2 b) {
  Rect2 result = rect2(a + b.min_x, a + b.min_y, a + b.max_x, a + b.max_y);
  return result;
}
api Rect2 &operator+=(Rect2 &a, Rect2 b) {
  a = a + b;
  return a;
}
api Rect2 &operator+=(Rect2 &a, F32 b) {
  a = a + b;
  return a;
}
api Rect2 operator-(Rect2 a, Rect2 b) {
  Rect2 result = rect2(a.min_x - b.min_x, a.min_y - b.min_y, a.max_x - b.max_x, a.max_y - b.max_y);
  return result;
}
api Rect2 operator-(Rect2 a, F32 b) {
  Rect2 result = rect2(a.min_x - b, a.min_y - b, a.max_x - b, a.max_y - b);
  return result;
}
api Rect2 operator-(F32 a, Rect2 b) {
  Rect2 result = rect2(a - b.min_x, a - b.min_y, a - b.max_x, a - b.max_y);
  return result;
}
api Rect2 &operator-=(Rect2 &a, Rect2 b) {
  a = a - b;
  return a;
}
api Rect2 &operator-=(Rect2 &a, F32 b) {
  a = a - b;
  return a;
}
api Rect2 operator*(Rect2 a, Rect2 b) {
  Rect2 result = rect2(a.min_x * b.min_x, a.min_y * b.min_y, a.max_x * b.max_x, a.max_y * b.max_y);
  return result;
}
api Rect2 operator*(Rect2 a, F32 b) {
  Rect2 result = rect2(a.min_x * b, a.min_y * b, a.max_x * b, a.max_y * b);
  return result;
}
api Rect2 operator*(F32 a, Rect2 b) {
  Rect2 result = rect2(a * b.min_x, a * b.min_y, a * b.max_x, a * b.max_y);
  return result;
}
api Rect2 &operator*=(Rect2 &a, Rect2 b) {
  a = a * b;
  return a;
}
api Rect2 &operator*=(Rect2 &a, F32 b) {
  a = a * b;
  return a;
}
api Rect2 operator/(Rect2 a, Rect2 b) {
  Rect2 result = rect2(a.min_x / b.min_x, a.min_y / b.min_y, a.max_x / b.max_x, a.max_y / b.max_y);
  return result;
}
api Rect2 operator/(Rect2 a, F32 b) {
  Rect2 result = rect2(a.min_x / b, a.min_y / b, a.max_x / b, a.max_y / b);
  return result;
}
api Rect2 operator/(F32 a, Rect2 b) {
  Rect2 result = rect2(a / b.min_x, a / b.min_y, a / b.max_x, a / b.max_y);
  return result;
}
api Rect2 &operator/=(Rect2 &a, Rect2 b) {
  a = a / b;
  return a;
}
api Rect2 &operator/=(Rect2 &a, F32 b) {
  a = a / b;
  return a;
}
api B32 operator!=(Rect2 a, Rect2 b) {
  B32 result = (a.min_x != b.min_x) || (a.min_y != b.min_y) || (a.max_x != b.max_x) || (a.max_y != b.max_y);
  return result;
}
api B32 operator==(Rect2 a, Rect2 b) {
  B32 result = (a.min_x == b.min_x) && (a.min_y == b.min_y) && (a.max_x == b.max_x) && (a.max_y == b.max_y);
  return result;
}
api Rect2 operator-(Rect2 a) {
  Rect2 result = rect2(-a.min_x, -a.min_y, -a.max_x, -a.max_y);
  return result;
}
api Rect2 clamp(Rect2 min, Rect2 val, Rect2 max) {
  Rect2 result = rect2(clamp(min.min_x, val.min_x, max.min_x), clamp(min.min_y, val.min_y, max.min_y),
                       clamp(min.max_x, val.max_x, max.max_x), clamp(min.max_y, val.max_y, max.max_y));
  return result;
}
api Rect2 clamp(F32 min, Rect2 val, F32 max) {
  Rect2 result = rect2(clamp(min, val.min_x, max), clamp(min, val.min_y, max), clamp(min, val.max_x, max),
                       clamp(min, val.max_y, max));
  return result;
}
api Rect2 clamp01(Rect2 val) {
  Rect2 result = rect2(clamp01(val.min_x), clamp01(val.min_y), clamp01(val.max_x), clamp01(val.max_y));
  return result;
}
api Rect2I rect2i(S32 min_x, S32 min_y, S32 max_x, S32 max_y) {
  Rect2I result;
  result.min_x = min_x;
  result.min_y = min_y;
  result.max_x = max_x;
  result.max_y = max_y;
  return result;
}
api Rect2I rect2i() { return rect2i(0, 0, 0, 0); }
api Rect2I operator+(Rect2I a, Rect2I b) {
  Rect2I result = rect2i(a.min_x + b.min_x, a.min_y + b.min_y, a.max_x + b.max_x, a.max_y + b.max_y);
  return result;
}
api Rect2I operator+(Rect2I a, S32 b) {
  Rect2I result = rect2i(a.min_x + b, a.min_y + b, a.max_x + b, a.max_y + b);
  return result;
}
api Rect2I operator+(S32 a, Rect2I b) {
  Rect2I result = rect2i(a + b.min_x, a + b.min_y, a + b.max_x, a + b.max_y);
  return result;
}
api Rect2I &operator+=(Rect2I &a, Rect2I b) {
  a = a + b;
  return a;
}
api Rect2I &operator+=(Rect2I &a, S32 b) {
  a = a + b;
  return a;
}
api Rect2I operator-(Rect2I a, Rect2I b) {
  Rect2I result = rect2i(a.min_x - b.min_x, a.min_y - b.min_y, a.max_x - b.max_x, a.max_y - b.max_y);
  return result;
}
api Rect2I operator-(Rect2I a, S32 b) {
  Rect2I result = rect2i(a.min_x - b, a.min_y - b, a.max_x - b, a.max_y - b);
  return result;
}
api Rect2I operator-(S32 a, Rect2I b) {
  Rect2I result = rect2i(a - b.min_x, a - b.min_y, a - b.max_x, a - b.max_y);
  return result;
}
api Rect2I &operator-=(Rect2I &a, Rect2I b) {
  a = a - b;
  return a;
}
api Rect2I &operator-=(Rect2I &a, S32 b) {
  a = a - b;
  return a;
}
api Rect2I operator*(Rect2I a, Rect2I b) {
  Rect2I result = rect2i(a.min_x * b.min_x, a.min_y * b.min_y, a.max_x * b.max_x, a.max_y * b.max_y);
  return result;
}
api Rect2I operator*(Rect2I a, S32 b) {
  Rect2I result = rect2i(a.min_x * b, a.min_y * b, a.max_x * b, a.max_y * b);
  return result;
}
api Rect2I operator*(S32 a, Rect2I b) {
  Rect2I result = rect2i(a * b.min_x, a * b.min_y, a * b.max_x, a * b.max_y);
  return result;
}
api Rect2I &operator*=(Rect2I &a, Rect2I b) {
  a = a * b;
  return a;
}
api Rect2I &operator*=(Rect2I &a, S32 b) {
  a = a * b;
  return a;
}
api Rect2I operator/(Rect2I a, Rect2I b) {
  Rect2I result = rect2i(a.min_x / b.min_x, a.min_y / b.min_y, a.max_x / b.max_x, a.max_y / b.max_y);
  return result;
}
api Rect2I operator/(Rect2I a, S32 b) {
  Rect2I result = rect2i(a.min_x / b, a.min_y / b, a.max_x / b, a.max_y / b);
  return result;
}
api Rect2I operator/(S32 a, Rect2I b) {
  Rect2I result = rect2i(a / b.min_x, a / b.min_y, a / b.max_x, a / b.max_y);
  return result;
}
api Rect2I &operator/=(Rect2I &a, Rect2I b) {
  a = a / b;
  return a;
}
api Rect2I &operator/=(Rect2I &a, S32 b) {
  a = a / b;
  return a;
}
api B32 operator!=(Rect2I a, Rect2I b) {
  B32 result = (a.min_x != b.min_x) || (a.min_y != b.min_y) || (a.max_x != b.max_x) || (a.max_y != b.max_y);
  return result;
}
api B32 operator==(Rect2I a, Rect2I b) {
  B32 result = (a.min_x == b.min_x) && (a.min_y == b.min_y) && (a.max_x == b.max_x) && (a.max_y == b.max_y);
  return result;
}
api Rect2I operator-(Rect2I a) {
  Rect2I result = rect2i(-a.min_x, -a.min_y, -a.max_x, -a.max_y);
  return result;
}
api Rect2I clamp(Rect2I min, Rect2I val, Rect2I max) {
  Rect2I result = rect2i(clamp(min.min_x, val.min_x, max.min_x), clamp(min.min_y, val.min_y, max.min_y),
                         clamp(min.max_x, val.max_x, max.max_x), clamp(min.max_y, val.max_y, max.max_y));
  return result;
}
api Rect2I clamp(S32 min, Rect2I val, S32 max) {
  Rect2I result = rect2i(clamp(min, val.min_x, max), clamp(min, val.min_y, max), clamp(min, val.max_x, max),
                         clamp(min, val.max_y, max));
  return result;
}
api Rect2I clamp01(Rect2I val) {
  Rect2I result = rect2i(clamp01(val.min_x), clamp01(val.min_y), clamp01(val.max_x), clamp01(val.max_y));
  return result;
}
api Vec2 cast_v2(Vec2I a) {
  Vec2 result = vec2((F32)(a.x), (F32)(a.y));
  return result;
}
api Vec2I cast_v2i(Vec2 a) {
  Vec2I result = vec2i((S32)(a.x), (S32)(a.y));
  return result;
}
api Vec2I round_cast_v2i(Vec2 a) {
  Vec2I result = vec2i((S32)round(a.x), (S32)round(a.y));
  return result;
}
api Vec2I ceil_cast_v2i(Vec2 a) {
  Vec2I result = vec2i((S32)ceil(a.x), (S32)ceil(a.y));
  return result;
}
api Vec2I floor_cast_v2i(Vec2 a) {
  Vec2I result = vec2i((S32)floor(a.x), (S32)floor(a.y));
  return result;
}
api Vec3 cast_v3(Vec3I a) {
  Vec3 result = vec3((F32)(a.x), (F32)(a.y), (F32)(a.z));
  return result;
}
api Vec3I cast_v3i(Vec3 a) {
  Vec3I result = vec3i((S32)(a.x), (S32)(a.y), (S32)(a.z));
  return result;
}
api Vec3I round_cast_v3i(Vec3 a) {
  Vec3I result = vec3i((S32)round(a.x), (S32)round(a.y), (S32)round(a.z));
  return result;
}
api Vec3I ceil_cast_v3i(Vec3 a) {
  Vec3I result = vec3i((S32)ceil(a.x), (S32)ceil(a.y), (S32)ceil(a.z));
  return result;
}
api Vec3I floor_cast_v3i(Vec3 a) {
  Vec3I result = vec3i((S32)floor(a.x), (S32)floor(a.y), (S32)floor(a.z));
  return result;
}
api Vec4 cast_v4(Vec4I a) {
  Vec4 result = vec4((F32)(a.x), (F32)(a.y), (F32)(a.z), (F32)(a.w));
  return result;
}
api Vec4I cast_v4i(Vec4 a) {
  Vec4I result = v4i((S32)(a.x), (S32)(a.y), (S32)(a.z), (S32)(a.w));
  return result;
}
api Vec4I round_cast_v4i(Vec4 a) {
  Vec4I result = v4i((S32)round(a.x), (S32)round(a.y), (S32)round(a.z), (S32)round(a.w));
  return result;
}
api Vec4I ceil_cast_v4i(Vec4 a) {
  Vec4I result = v4i((S32)ceil(a.x), (S32)ceil(a.y), (S32)ceil(a.z), (S32)ceil(a.w));
  return result;
}
api Vec4I floor_cast_v4i(Vec4 a) {
  Vec4I result = v4i((S32)floor(a.x), (S32)floor(a.y), (S32)floor(a.z), (S32)floor(a.w));
  return result;
}
api Rect2 cast_rect2(Rect2I a) {
  Rect2 result = rect2((F32)(a.min_x), (F32)(a.min_y), (F32)(a.max_x), (F32)(a.max_y));
  return result;
}
api Rect2I cast_rect2i(Rect2 a) {
  Rect2I result = rect2i((S32)(a.min_x), (S32)(a.min_y), (S32)(a.max_x), (S32)(a.max_y));
  return result;
}
api Rect2I round_cast_rect2i(Rect2 a) {
  Rect2I result = rect2i((S32)round(a.min_x), (S32)round(a.min_y), (S32)round(a.max_x), (S32)round(a.max_y));
  return result;
}
api Rect2I ceil_cast_rect2i(Rect2 a) {
  Rect2I result = rect2i((S32)ceil(a.min_x), (S32)ceil(a.min_y), (S32)ceil(a.max_x), (S32)ceil(a.max_y));
  return result;
}
api Rect2I floor_cast_rect2i(Rect2 a) {
  Rect2I result = rect2i((S32)floor(a.min_x), (S32)floor(a.min_y), (S32)floor(a.max_x), (S32)floor(a.max_y));
  return result;
}
api Rect2I rect2i(Vec2I a, Vec2I b) {
  Rect2I result = rect2i(a.p[0], a.p[1], b.p[0], b.p[1]);
  return result;
}
api Rect2I rect2i(S32 a, Vec3I b) {
  Rect2I result = rect2i(a, b.p[0], b.p[1], b.p[2]);
  return result;
}
api Rect2I rect2i(Vec3I a, S32 b) {
  Rect2I result = rect2i(a.p[0], a.p[1], a.p[2], b);
  return result;
}
api Rect2I rect2i(Vec2I a, S32 b, S32 c) {
  Rect2I result = rect2i(a.p[0], a.p[1], b, c);
  return result;
}
api Rect2I rect2i(S32 a, S32 b, Vec2I c) {
  Rect2I result = rect2i(a, b, c.p[0], c.p[1]);
  return result;
}
api Vec4I v4i(Vec2I a, Vec2I b) {
  Vec4I result = v4i(a.p[0], a.p[1], b.p[0], b.p[1]);
  return result;
}
api Vec4I v4i(S32 a, Vec3I b) {
  Vec4I result = v4i(a, b.p[0], b.p[1], b.p[2]);
  return result;
}
api Vec4I v4i(Vec3I a, S32 b) {
  Vec4I result = v4i(a.p[0], a.p[1], a.p[2], b);
  return result;
}
api Vec4I v4i(Vec2I a, S32 b, S32 c) {
  Vec4I result = v4i(a.p[0], a.p[1], b, c);
  return result;
}
api Vec4I v4i(S32 a, S32 b, Vec2I c) {
  Vec4I result = v4i(a, b, c.p[0], c.p[1]);
  return result;
}
api Rect2 rect2(Vec2 a, Vec2 b) {
  Rect2 result = rect2(a.p[0], a.p[1], b.p[0], b.p[1]);
  return result;
}
api Rect2 rect2(F32 a, Vec3 b) {
  Rect2 result = rect2(a, b.p[0], b.p[1], b.p[2]);
  return result;
}
api Rect2 rect2(Vec3 a, F32 b) {
  Rect2 result = rect2(a.p[0], a.p[1], a.p[2], b);
  return result;
}
api Rect2 rect2(Vec2 a, F32 b, F32 c) {
  Rect2 result = rect2(a.p[0], a.p[1], b, c);
  return result;
}
api Rect2 rect2(F32 a, F32 b, Vec2 c) {
  Rect2 result = rect2(a, b, c.p[0], c.p[1]);
  return result;
}
api Vec4 vec4(Vec2 a, Vec2 b) {
  Vec4 result = vec4(a.p[0], a.p[1], b.p[0], b.p[1]);
  return result;
}
api Vec4 vec4(F32 a, Vec3 b) {
  Vec4 result = vec4(a, b.p[0], b.p[1], b.p[2]);
  return result;
}
api Vec4 vec4(Vec3 a, F32 b) {
  Vec4 result = vec4(a.p[0], a.p[1], a.p[2], b);
  return result;
}
api Vec4 vec4(Vec2 a, F32 b, F32 c) {
  Vec4 result = vec4(a.p[0], a.p[1], b, c);
  return result;
}
api Vec4 vec4(F32 a, F32 b, Vec2 c) {
  Vec4 result = vec4(a, b, c.p[0], c.p[1]);
  return result;
}
api Rect2 rect2_min_size(Vec2 a, Vec2 b) {
  Rect2 result = rect2(a.p[0], a.p[1], b.p[0], b.p[1]);
  result.max += result.min;
  return result;
}
api Rect2 rect2_min_size(F32 a, Vec3 b) {
  Rect2 result = rect2(a, b.p[0], b.p[1], b.p[2]);
  result.max += result.min;
  return result;
}
api Rect2 rect2_min_size(Vec3 a, F32 b) {
  Rect2 result = rect2(a.p[0], a.p[1], a.p[2], b);
  result.max += result.min;
  return result;
}
api Rect2 rect2_min_size(Vec2 a, F32 b, F32 c) {
  Rect2 result = rect2(a.p[0], a.p[1], b, c);
  result.max += result.min;
  return result;
}
api Rect2 rect2_min_size(F32 a, F32 b, Vec2 c) {
  Rect2 result = rect2(a, b, c.p[0], c.p[1]);
  result.max += result.min;
  return result;
}
api Rect2I rect2i_min_size(Vec2I a, Vec2I b) {
  Rect2I result = rect2i(a.p[0], a.p[1], b.p[0], b.p[1]);
  result.max += result.min;
  return result;
}
api Rect2I rect2i_min_size(S32 a, Vec3I b) {
  Rect2I result = rect2i(a, b.p[0], b.p[1], b.p[2]);
  result.max += result.min;
  return result;
}
api Rect2I rect2i_min_size(Vec3I a, S32 b) {
  Rect2I result = rect2i(a.p[0], a.p[1], a.p[2], b);
  result.max += result.min;
  return result;
}
api Rect2I rect2i_min_size(Vec2I a, S32 b, S32 c) {
  Rect2I result = rect2i(a.p[0], a.p[1], b, c);
  result.max += result.min;
  return result;
}
api Rect2I rect2i_min_size(S32 a, S32 b, Vec2I c) {
  Rect2I result = rect2i(a, b, c.p[0], c.p[1]);
  result.max += result.min;
  return result;
}
api Rect2 rect2_center_half_dim(Vec2 a, Vec2 b) {
  Rect2 result = rect2(a.p[0], a.p[1], b.p[0], b.p[1]);
  Vec2 center = result.min;
  result.min -= result.max;
  result.max += center;
  return result;
}
api Rect2 rect2_center_half_dim(F32 a, Vec3 b) {
  Rect2 result = rect2(a, b.p[0], b.p[1], b.p[2]);
  Vec2 center = result.min;
  result.min -= result.max;
  result.max += center;
  return result;
}
api Rect2 rect2_center_half_dim(Vec3 a, F32 b) {
  Rect2 result = rect2(a.p[0], a.p[1], a.p[2], b);
  Vec2 center = result.min;
  result.min -= result.max;
  result.max += center;
  return result;
}
api Rect2 rect2_center_half_dim(Vec2 a, F32 b, F32 c) {
  Rect2 result = rect2(a.p[0], a.p[1], b, c);
  Vec2 center = result.min;
  result.min -= result.max;
  result.max += center;
  return result;
}
api Rect2 rect2_center_half_dim(F32 a, F32 b, Vec2 c) {
  Rect2 result = rect2(a, b, c.p[0], c.p[1]);
  Vec2 center = result.min;
  result.min -= result.max;
  result.max += center;
  return result;
}
api Rect2I rect2i_center_half_dim(Vec2I a, Vec2I b) {
  Rect2I result = rect2i(a.p[0], a.p[1], b.p[0], b.p[1]);
  Vec2I center = result.min;
  result.min -= result.max;
  result.max += center;
  return result;
}
api Rect2I rect2i_center_half_dim(S32 a, Vec3I b) {
  Rect2I result = rect2i(a, b.p[0], b.p[1], b.p[2]);
  Vec2I center = result.min;
  result.min -= result.max;
  result.max += center;
  return result;
}
api Rect2I rect2i_center_half_dim(Vec3I a, S32 b) {
  Rect2I result = rect2i(a.p[0], a.p[1], a.p[2], b);
  Vec2I center = result.min;
  result.min -= result.max;
  result.max += center;
  return result;
}
api Rect2I rect2i_center_half_dim(Vec2I a, S32 b, S32 c) {
  Rect2I result = rect2i(a.p[0], a.p[1], b, c);
  Vec2I center = result.min;
  result.min -= result.max;
  result.max += center;
  return result;
}
api Rect2I rect2i_center_half_dim(S32 a, S32 b, Vec2I c) {
  Rect2I result = rect2i(a, b, c.p[0], c.p[1]);
  Vec2I center = result.min;
  result.min -= result.max;
  result.max += center;
  return result;
}
api Rect2 rect2_min_size(F32 a, F32 b, F32 c, F32 d) {
  Rect2 result = rect2(a, b, c, d);
  result.max += result.min;
  return result;
}
api Rect2I rect2i_min_size(S32 a, S32 b, S32 c, S32 d) {
  Rect2I result = rect2i(a, b, c, d);
  result.max += result.min;
  return result;
}
api Rect2 rect2_center_half_dim(F32 a, F32 b, F32 c, F32 d) {
  Rect2 result = rect2(a, b, c, d);
  Vec2 center = result.min;
  result.min -= result.max;
  result.max += center;
  return result;
}
api Rect2I rect2i_center_half_dim(S32 a, S32 b, S32 c, S32 d) {
  Rect2I result = rect2i(a, b, c, d);
  Vec2I center = result.min;
  result.min -= result.max;
  result.max += center;
  return result;
}
api Vec3 vec3(F32 a, Vec2 b) {
  Vec3 result = vec3(a, b.p[0], b.p[1]);
  return result;
}
api Vec3 vec3(Vec2 a, F32 b) {
  Vec3 result = vec3(a.p[0], a.p[1], b);
  return result;
}
api Vec3I vec3i(S32 a, Vec2I b) {
  Vec3I result = vec3i(a, b.p[0], b.p[1]);
  return result;
}
api Vec3I vec3i(Vec2I a, S32 b) {
  Vec3I result = vec3i(a.p[0], a.p[1], b);
  return result;
}

api Rect2I intersect(Rect2I a, Rect2I clip) {
  Rect2I result;
  result.min.x = max(a.min.x, clip.min.x);
  result.min.y = max(a.min.y, clip.min.y);
  result.max.x = min(a.max.x, clip.max.x);
  result.max.y = min(a.max.y, clip.max.y);
  return result;
}

api B32 has_area(Rect2I a) {
  B32 result = (a.max_x - a.min_x > 0) && (a.max_y - a.min_y > 0);
  return result;
}

api Rect2 intersect(Rect2 a, Rect2 clip) {
  Rect2 result;
  result.min.x = max(a.min.x, clip.min.x);
  result.min.y = max(a.min.y, clip.min.y);
  result.max.x = min(a.max.x, clip.max.x);
  result.max.y = min(a.max.y, clip.max.y);
  return result;
}

api B32 has_area(Rect2 a) {
  B32 result = (a.max_x - a.min_x > 0) && (a.max_y - a.min_y > 0);
  return result;
}

api Vec2 perp(Vec2 a) {
  Vec2 result = vec2(-a.y, a.x);
  return result;
}

api Vec4 vec4argb(U32 v) {
  U8 a = (v >> 24) & 0x000000ff;
  U8 r = (v >> 16) & 0x000000ff;
  U8 g = (v >> 8) & 0x000000ff;
  U8 b = (v >> 0) & 0x000000ff;
  Vec4 result = vec4((F32)r / 255.f, (F32)g / 255.f, (F32)b / 255.f, (F32)a / 255.f);
  return result;
}

api Vec4 vec4abgr(U32 c) {
  float a = ((c & 0xff000000) >> 24) / 255.f;
  float b = ((c & 0x00ff0000) >> 16) / 255.f;
  float g = ((c & 0x0000ff00) >> 8) / 255.f;
  float r = ((c & 0x000000ff) >> 0) / 255.f;
  Vec4 result = vec4(r, g, b, a);
  return result;
}

api U32 vec4_to_u32argb(Vec4 c) {
  U32 result = (U32)((U8)(c.a * 255.f) << 24u | (U8)(c.r * 255.f) << 16u | (U8)(c.g * 255.f) << 8u |
                     (U8)(c.b * 255.f) << 0u);
  return result;
}

api U32 vec4_to_u32abgr(Vec4 color) {
  U8 red = (U8)(color.r * 255);
  U8 green = (U8)(color.g * 255);
  U8 blue = (U8)(color.b * 255);
  U8 alpha = (U8)(color.a * 255);
  U32 result = (U32)(alpha << 24 | blue << 16 | green << 8 | red << 0);
  return result;
}

api Mat4 operator*(Mat4 a, Mat4 b) {
  Mat4 result;
  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 4; x++) {
      result.p[y][x] =
        a.p[y][0] * b.p[0][x] + a.p[y][1] * b.p[1][x] + a.p[y][2] * b.p[2][x] + a.p[y][3] * b.p[3][x];
    }
  }
  return result;
}

api Vec4 operator*(Mat4 a, Vec4 b) {
  Vec4 result;
  for (int y = 0; y < 4; y++) {
    result.p[y] = a.p[y][0] * b.p[0] + a.p[y][1] * b.p[1] + a.p[y][2] * b.p[2] + a.p[y][3] * b.p[3];
  }
  return result;
}

api Vec3 operator*(Mat4 a, Vec3 b) {
  Vec4 result;
  for (int y = 0; y < 4; y++)
    result.p[y] = a.p[y][0] * b.p[0] + a.p[y][1] * b.p[1] + a.p[y][2] * b.p[2] + a.p[y][3] * 1;
  return result.xyz;
}

api Vec3 cross(Vec3 a, Vec3 b) {
  Vec3 result;
  result.x = a.y * b.z - a.z * b.y;
  result.y = a.z * b.x - a.x * b.z;
  result.z = a.x * b.y - a.y * b.x;
  return result;
}

api Vec2 normalize(Vec2 a) {
  Vec2 result = {};
  F32 len = length(a);
  if (len != 0.f) {
    F32 inv_len = 1.0f / len;
    result.x = a.x * inv_len;
    result.y = a.y * inv_len;
  }
  return result;
}

api Vec3 normalize(Vec3 a) {
  Vec3 result = {};
  F32 len = length(a);
  if (len != 0.f) {
    F32 inv_len = 1.0f / len;
    result.x = a.x * inv_len;
    result.y = a.y * inv_len;
    result.z = a.z * inv_len;
  }
  return result;
}

api Vec4 normalize(Vec4 a) {
  Vec4 result = {};
  F32 len = length(a);
  if (len != 0.f) {
    F32 inv_len = 1.0f / len;
    result.x = a.x * inv_len;
    result.y = a.y * inv_len;
    result.z = a.z * inv_len;
    result.w = a.w * inv_len;
  }
  return result;
}

function
Mat4 mat4_identity() {
  Mat4 result = {};
  result.p[0][0] = 1;
  result.p[1][1] = 1;
  result.p[2][2] = 1;
  result.p[3][3] = 1;
  return result;
}

function
Mat4 mat4_scale(Vec3 a) {
  Mat4 result = {};
  result.p[0][0] = a.x;
  result.p[1][1] = a.y;
  result.p[2][2] = a.z;
  result.p[3][3] = 1;
  return result;
}

function
Mat4 mat4_translation(Vec3 a) {
  return {
    1, 0, 0, a.x,
    0, 1, 0, a.y,
    0, 0, 1, a.z,
    0, 0, 0, 1
  };
}

function
Mat4 mat4_rotation_z(float rotation) {
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

function
Mat4 mat4_rotation_y(float rotation) {
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

function
Mat4 mat4_rotation_x(float rotation) {
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

constexpr F32 deg2rad = (PI32 / 180.f); // @Usage: degree * deg2rad = radians;
constexpr F32 rad2deg = (180.f / PI32);
function
Mat4 mat4_perspective(float fov, float window_x, float window_y, float znear, float zfar) {
  float aspect_ratio = window_y / window_x;
  float f = (1.f / tanf((fov/2.f)*deg2rad));
  Mat4 result = {
    aspect_ratio*f, 0, 0, 0,
    0, f, 0, 0,
    0, 0, (zfar)-(zfar-znear),(-zfar*znear)-(zfar - znear),
    0,0,1,0
  };
  return result;
}

function Mat4 mat4_look_at(Vec3 pos, Vec3 target, Vec3 up) {
  Vec3 z = normalize(target - pos);
  Vec3 x = normalize(cross(up, z));
  Vec3 y = cross(z, x);
  Mat4 result = {
    x.x,x.y,x.z,-dot(x,pos),
    y.x,y.y,y.z,-dot(y,pos),
    z.x,z.y,z.z,-dot(z,pos),
    0,0,0,       1,
  };
  return result;
}

function
Mat4 mat4_transpose(Mat4 a) {
  Mat4 result = a;
  result.p[0][1] = result.p[1][0];
  result.p[0][2] = result.p[2][0];
  result.p[0][3] = result.p[3][0];
  result.p[2][1] = result.p[1][2];
  result.p[3][1] = result.p[1][3];
  result.p[3][2] = result.p[2][3];
  return result;
}

function
Mat4 mat4_translate(Mat4 a, Vec3 translation) {
  a.p[0][0] += translation.x;
  a.p[0][1] += translation.y;
  a.p[0][2] += translation.z;
  return a;
}