#include <intrin.h>

union Vec8{
  __m256 simd;
  F32    e[8];
  Vec4   v4[2];
  force_inline F32 &operator[](S64 i){ return e[i]; }
};

force_inline Vec8 floor8(Vec8 v){ return {_mm256_floor_ps(v.simd)}; }
force_inline Vec8 loadu8(void *m){ return {_mm256_loadu_ps((const float *)m)}; }
force_inline Vec8 vec8(F32 x){return {_mm256_set1_ps(x)}; }
force_inline Vec8 vec8(F32 a, F32 b, F32 c, F32 d, F32 e, F32 f, F32 g, F32 h){ return {_mm256_set_ps(h, g, f, e, d, c, b, a)}; }
Vec8 operator+(Vec8 a, Vec8 b){ return {_mm256_add_ps(a.simd, b.simd)}; }
Vec8 operator-(Vec8 a, Vec8 b){ return {_mm256_sub_ps(a.simd, b.simd)}; }
Vec8 operator*(Vec8 a, Vec8 b){ return {_mm256_mul_ps(a.simd, b.simd)}; }
Vec8 operator/(Vec8 a, Vec8 b){ return {_mm256_div_ps(a.simd, b.simd)}; }

Vec8 operator>=(Vec8 a, Vec8 b){ return {_mm256_cmp_ps(a.simd, b.simd, _CMP_GE_OQ)}; }
Vec8 operator<=(Vec8 a, Vec8 b){ return {_mm256_cmp_ps(a.simd, b.simd, _CMP_LE_OQ)}; }
Vec8 operator<(Vec8 a, Vec8 b){ return {_mm256_cmp_ps(a.simd, b.simd, _CMP_LT_OQ)}; }
Vec8 operator>(Vec8 a, Vec8 b){ return {_mm256_cmp_ps(a.simd, b.simd, _CMP_GT_OQ)}; }
Vec8 operator&(Vec8 a, Vec8 b){ return {_mm256_and_ps(a.simd, b.simd)}; }

Vec8 operator+=(Vec8 &a, Vec8 b){ a = a + b; return a; }
Vec8 operator-=(Vec8 &a, Vec8 b){ a = a - b; return a; }
Vec8 operator*=(Vec8 &a, Vec8 b){ a = a * b; return a; }
Vec8 operator/=(Vec8 &a, Vec8 b){ a = a / b; return a; }

union Vec8I{
  __m256i simd;
  S32     e[8];
  Vec4I   v4[2];
  force_inline S32 &operator[](S64 i){ return e[i]; }
};

Vec8I vec8i(S32 x){return {_mm256_set1_epi32(x)}; }
Vec8I vec8i(S32 a, S32 b, S32 c, S32 d, S32 e, S32 f, S32 g, S32 h){ return {_mm256_set_epi32(h, g, f, e, d, c, b, a)}; }
Vec8I operator+(Vec8I a, Vec8I b){ return {_mm256_add_epi32(a.simd, b.simd)}; }
Vec8I operator-(Vec8I a, Vec8I b){ return {_mm256_sub_epi32(a.simd, b.simd)}; }
Vec8I operator*(Vec8I a, Vec8I b){ return {_mm256_mul_epi32(a.simd, b.simd)}; }
// Vec8I operator/(Vec8I a, Vec8I b){ return {_mm256_div_epi32(a.simd, b.simd)}; }
Vec8I operator+=(Vec8I &a, Vec8I b){ return a + b; }

Vec8I convert_vec8_to_vec8i(Vec8 v){ return Vec8I{_mm256_cvtps_epi32(v.simd)}; }
