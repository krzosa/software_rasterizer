#include <intrin.h>

union Vec8{
  __m256 simd;
  F32    e[8];
  Vec4   v4[2];
};


Vec8 vec8(F32 x){return {_mm256_set1_ps(x)}; }
Vec8 vec8(F32 a, F32 b, F32 c, F32 d, F32 e, F32 f, F32 g, F32 h){ return {_mm256_set_ps(a,b,c,d,e,f,g,h)}; }
Vec8 operator+(Vec8 a, Vec8 b){ return {_mm256_add_ps(a.simd, b.simd)}; }
Vec8 operator-(Vec8 a, Vec8 b){ return {_mm256_sub_ps(a.simd, b.simd)}; }
Vec8 operator*(Vec8 a, Vec8 b){ return {_mm256_mul_ps(a.simd, b.simd)}; }
Vec8 operator/(Vec8 a, Vec8 b){ return {_mm256_div_ps(a.simd, b.simd)}; }
Vec8 operator+=(Vec8 &a, Vec8 b){ return a + b; }