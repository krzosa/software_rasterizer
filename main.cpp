/////////////////////////////////////////////////////////////////////////////////////
///
/// ### Things to do:
///
/// - [x] Drawing triangles
/// - [x] Drawing cubes and lines for testing
/// - [x] Y up coordinate system, left handed
/// - [x] Drawing a cube with perspective
/// - [x] Culling triangles facing away from camera
/// - [x] Texture mapping
/// - [x] Basic linear transformations - rotation, translation, scaling
/// - [x] Bilinear filtering of textures
/// - [x] Nearest filtering
/// - [x] Fix the gaps between triangles (it also improved look of triangle edges)
/// - [ ] Perspective matrix vs simple perspective
/// - [x] Perspective correct interpolation
/// - [x] Depth buffer
/// - [x] Gamma correct blending - converting to almost linear space
/// - [x] Alpha blending
/// - [x] Premultiplied alpha
/// - [x] Merge with base
/// - [ ] Fill convention
/// - [ ] Antialiasing (seems like performance gets really bad with this)
/// - [x] LookAt Camera
/// - [x] FPS Camera
/// - [ ] Quarternions for rotations
/// - [x] Reading OBJ models
/// - [x] Dumping raw obj files
/// - [x] Loading raw obj files, big startup speedup!
/// - [ ] Reading more OBJ formats
/// - [x] Reading OBJ .mtl files
/// - [x] Loading materials
/// - [x] Rendering textures obj models
/// - [x] Reading complex obj models (sponza)
/// - [x] Fix sponza uv coordinates - the issue was uv > 1 and uv < 0
/// - [x] Clipping
///   - [x] Triagnle rectangle bound clipping
///   - [x] A way of culling Z out triangles
///     - [x] Simple test z clipping
///     - [x] Maybe should clip a triangle on znear zfar plane?
///     - [x] Maybe should clip out triangles that are fully z out before draw_triangle
/// - [ ] Effects!!!
///   - [ ] Outlines
/// - [ ] Lightning
///   - [ ] Proper normal interpolation
///     * `https://hero.handmade.network/episode/code/day101/#105
///   - [ ] Phong
///     - [x] diffuse
///     - [x] ambient
///     - [ ] specular
///      * reflecting vectors
///   - [ ] Use all materials from OBJ
///   - [ ] Point light
/// - [ ] Reading PMX files
/// - [ ] Rendering multiple objects, queue renderer
///   - [x] Simple function to render a mesh
/// - [x] Simple profiling tooling
/// - [x] Statistics based on profiler data
/// - [x] Find cool profilers - ExtraSleepy, Vtune
/// - [ ] Optimizations
///   - [ ] Inline edge function
///   - [ ] Expand edge functions to more optimized version
///   - [ ] Test 4x2 bitmap layout?
///   - [ ] Edge function to integer
///   - [ ] Use integer bit operations to figure out if plus. (edge0|edge1|edge2)>=0
///   - [ ] SIMD
///   - [ ] Multithreading
///
/// - [x] Text rendering
/// - [ ] UI
///   - [x] Labels
///   - [x] Settings variables
///   - [x] Signals
///   - [ ] Sliders
///   - [ ] Groups
/// - [x] Gamma correct alpha blending for rectangles and bitmaps
/// - [ ] Plotting of profile data
///    - [x] Simple scatter plot
///
///
/// ### Urgent:
///
/// - [ ] Simplify the code, especially for the 2d routines
/// - [x] Asset processor as second program
///
///
#if 0
#include "tracy/Tracy.hpp"
#undef assert
#endif


#include "multimedia.cpp"
#include "profile.cpp"
#include "obj.cpp"
#include "vec.cpp"

struct Vertex {
  Vec3 pos;
  Vec2 tex;
  Vec3 norm;
};

struct Render {
  Mat4 camera;
  Mat4 projection;
  Mat4 transform;

  Vec3 camera_pos;
  Vec3 camera_direction;
  Vec3 camera_forward_velocity;
  Vec2 camera_yaw;
  Vec3 camera_target;
  Bitmap img;
  B32 plot_ready;
  Bitmap plot;
  Bitmap screen320;
  F32 *depth320;
};

enum Scene {
  Scene_F22,
  Scene_Sponza,
  Scene_Count,
};

global F32 light_rotation = 0;
global F32 zfar_value = 100000.f;

function
Vec4 srgb_to_almost_linear(Vec4 a) {
  Vec4 result = {a.r*a.r, a.g*a.g, a.b*a.b, a.a};
  return result; // @Note: Linear would be to power of 2.2
}

function
Vec4 almost_linear_to_srgb(Vec4 a) {
  Vec4 result = { sqrtf(a.r), sqrtf(a.g), sqrtf(a.b), a.a };
  return result;
}

function
Vec4 premultiplied_alpha(Vec4 dst, Vec4 src) {
  Vec4 result;
  result.r = src.r + ((1-src.a) * dst.r);
  result.g = src.g + ((1-src.a) * dst.g);
  result.b = src.b + ((1-src.a) * dst.b);
  result.a = src.a + dst.a - src.a*dst.a;
  return result;
}

function
void draw_rect(Bitmap* dst, F32 X, F32 Y, F32 w, F32 h, Vec4 color) {
  int max_x = (int)(min(X + w, (F32)dst->x) + 0.5f);
  int max_y = (int)(min(Y + h, (F32)dst->y) + 0.5f);
  int min_x = (int)(max(0.f, X) + 0.5f);
  int min_y = (int)(max(0.f, Y) + 0.5f);

  color.rgb *= color.a;
  color = srgb_to_almost_linear(color);
  for (int y = min_y; y < max_y; y++) {
    for (int x = min_x; x < max_x; x++) {
      U32 *dst_pixel = dst->pixels + (x + y * dst->x);
      Vec4 dstc = srgb_to_almost_linear(vec4abgr(*dst_pixel));
      dstc = premultiplied_alpha(dstc, color);
      U32 color32 = vec4_to_u32abgr(almost_linear_to_srgb(dstc));
      *dst_pixel = color32;
    }
  }
}

function void
draw_bitmap(Bitmap *dst, Bitmap *src, Vec2 pos){
  S64 minx = (S64)(pos.x + 0.5);
  S64 miny = (S64)(pos.y + 0.5);
  S64 maxx = minx + src->x;
  S64 maxy = miny + src->y;
  S64 offsetx = 0;
  S64 offsety = 0;

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
  for (S64 y = miny; y < maxy; y++) {
    for (S64 x = minx; x < maxx; x++) {
      S64 tx = x - minx + offsetx;
      S64 ty = y - miny + offsety;
      U32 *dst_pixel = dst->pixels + (x + y * dst->x);
      U32 *pixel     = src->pixels + (tx + ty * src->x);
      Vec4 result_color = srgb_to_almost_linear(vec4abgr(*pixel));
      Vec4 dst_color = srgb_to_almost_linear(vec4abgr(*dst_pixel));
      result_color = premultiplied_alpha(dst_color, result_color);
      result_color = almost_linear_to_srgb(result_color);
      U32 color32 = vec4_to_u32abgr(result_color);
      *dst_pixel = color32;
    }
  }
}

function
void draw_bitmap(Bitmap* dst, Bitmap* src, Vec2 pos, Vec2 size) {
  S64 minx = (S64)(pos.x + 0.5);
  S64 miny = (S64)(pos.y + 0.5);
  S64 maxx = minx + (S64)(size.x + 0.5f);
  S64 maxy = miny + (S64)(size.y + 0.5f);
  S64 offsetx = 0;
  S64 offsety = 0;
  maxx = clamp_top(maxx, (S64)dst->x);
  maxy = clamp_top(maxy, (S64)dst->y);
  if (minx < 0) {
    offsetx = -minx;
    minx = 0;
  }
  if (miny < 0) {
    offsety = -miny;
    miny = 0;
  }

  F32 distx = (F32)(maxx - minx);
  F32 disty = (F32)(maxy - miny);
  for (S64 y = miny; y < maxy; y++) {
    for (S64 x = minx; x < maxx; x++) {
      F32 u = (F32)(x - minx) / distx;
      F32 v = (F32)(y - miny) / disty;
      S64 tx = (S64)(u * src->x + 0.5f);
      S64 ty = (S64)(v * src->y + 0.5f);
      U32 *dst_pixel = dst->pixels + (x + y * dst->x);
      U32 *pixel     = src->pixels + (tx + ty * src->x);
      Vec4 result_color = srgb_to_almost_linear(vec4abgr(*pixel));
      Vec4 dst_color = srgb_to_almost_linear(vec4abgr(*dst_pixel));
      result_color = premultiplied_alpha(dst_color, result_color);
      result_color = almost_linear_to_srgb(result_color);
      U32 color32 = vec4_to_u32abgr(result_color);
      *dst_pixel = color32;
    }
  }
}

function
Vec4 base_string(Bitmap *dst, Font *font, String word, Vec2 pos, B32 draw) {
  Vec2 og_position = pos;
  F32 max_x = pos.x;
  for (U64 i = 0; i < word.len; i++) {
    if (word.str[i] == ' ') {
      FontGlyph* g = &font->glyphs['_' - '!'];
      pos.x += g->xadvance;
      if (pos.x > max_x) max_x = pos.x;
    }
    else if (word.str[i] == '\n') {
      pos.y -= font->line_advance;
      pos.x = og_position.x;
    }
    else if((word.str[i] >= '!' && word.str[i] <= 127)){
      FontGlyph* g = &font->glyphs[word.str[i] - '!'];
      if(draw) draw_bitmap(dst, &g->bitmap, pos - g->bitmap.align);
      pos.x += g->xadvance;
      if (pos.x > max_x) max_x = pos.x;
    }
  }
  Vec4 rect = vec4(og_position.x, pos.y, max_x - og_position.x, og_position.y - pos.y + font->line_advance);
  return rect;
}

function
Vec4 draw_string(Bitmap *dst, Font *font, String word, Vec2 pos) {
  return base_string(dst, font, word, pos, true);
}

function
Vec4 get_string_rect(Font *font, String word, Vec2 pos) {
  return base_string(0, font, word, pos, false);
}

function
F32 edge_function(Vec4 vecp0, Vec4 vecp1, Vec4 p) {
  F32 result = (vecp1.y - vecp0.y) * (p.x - vecp0.x) - (vecp1.x - vecp0.x) * (p.y - vecp0.y);
  return result;
}

U64 filled_pixel_count;
U64 filled_pixel_total_time;
// #include "optimization_log.cpp"

#define I(x,i) (((F32 *)&x)[i])
#define Is(x,i) (((S32 *)&x)[i])
typedef __m256  F32x8;
typedef __m256i S32x8;
function
void draw_triangle_nearest(Bitmap* dst, F32 *depth_buffer, Bitmap *src, Vec3 light_direction,
                           Vec4 p0,   Vec4 p1,   Vec4 p2,
                           Vec2 tex0, Vec2 tex1, Vec2 tex2,
                           Vec3 norm0, Vec3 norm1, Vec3 norm2) {
  if(src->pixels == 0) return;

  PROFILE_SCOPE(draw_triangle);

  F32 min_x1 = (F32)(min(p0.x, min(p1.x, p2.x)));
  F32 min_y1 = (F32)(min(p0.y, min(p1.y, p2.y)));
  F32 max_x1 = (F32)(max(p0.x, max(p1.x, p2.x)));
  F32 max_y1 = (F32)(max(p0.y, max(p1.y, p2.y)));

  S64 min_x = (S64)max(0.f, floor(min_x1));
  S64 min_y = (S64)max(0.f, floor(min_y1));
  S64 max_x = (S64)min((F32)dst->x, ceil(max_x1));
  S64 max_y = (S64)min((F32)dst->y, ceil(max_y1));

  if (min_y >= max_y) return;
  if (min_x >= max_x) return;

  F32 dy10 = (p1.y - p0.y);
  F32 dy21 = (p2.y - p1.y);
  F32 dy02 = (p0.y - p2.y);

  F32 dx10 = (p1.x - p0.x);
  F32 dx21 = (p2.x - p1.x);
  F32 dx02 = (p0.x - p2.x);

  F32 C0 = dy10 * (p0.x) - dx10 * (p0.y);
  F32 C1 = dy21 * (p1.x) - dx21 * (p1.y);
  F32 C2 = dy02 * (p2.x) - dx02 * (p2.y);

  F32 Cy0 = dy10 * min_x - dx10 * min_y - C0;
  F32 Cy1 = dy21 * min_x - dx21 * min_y - C1;
  F32 Cy2 = dy02 * min_x - dx02 * min_y - C2;

  F32x8 var255 = _mm256_set1_ps(255);
  F32x8 var0 = _mm256_set1_ps(0);
  F32x8 var_max_x = _mm256_set1_ps(max_x);
  F32x8 var07 = _mm256_set_ps(7,6,5,4,3,2,1,0);
  // F32x8 var1 = _mm256_set1_ps(1);

  Vec8 var1 = vec8(1);
  Vec8I var0i = vec8i(0);
  Vec8I var1i = vec8i(1);
  Vec8 var1_8 = vec8(1,2,3,4,5,6,7,8);
  Vec8 Dy10 = vec8(dy10) * var1_8;
  Vec8 Dy21 = vec8(dy21) * var1_8;
  Vec8 Dy02 = vec8(dy02) * var1_8;

  F32x8 var_src_x_minus_one = _mm256_set1_ps(src->x-1);
  F32x8 var_src_y_minus_one = _mm256_set1_ps(src->y-1);

  F32x8 var_tex0x = _mm256_set1_ps(tex0.x);
  F32x8 var_tex1x = _mm256_set1_ps(tex1.x);
  F32x8 var_tex2x = _mm256_set1_ps(tex2.x);
  F32x8 var_tex0y = _mm256_set1_ps(tex0.y);
  F32x8 var_tex1y = _mm256_set1_ps(tex1.y);
  F32x8 var_tex2y = _mm256_set1_ps(tex2.y);

  F32x8 var_p0w = _mm256_set1_ps(p0.w);
  F32x8 var_p1w = _mm256_set1_ps(p1.w);
  F32x8 var_p2w = _mm256_set1_ps(p2.w);
  F32x8 one_over_p0w = _mm256_set1_ps(1.f / p0.w);
  F32x8 one_over_p1w = _mm256_set1_ps(1.f / p1.w);
  F32x8 one_over_p2w = _mm256_set1_ps(1.f / p2.w);

  U32 *destination = dst->pixels + dst->x*min_y;
  F32 area = (p1.y - p0.y) * (p2.x - p0.x) - (p1.x - p0.x) * (p2.y - p0.y);
  F32x8 area8 = _mm256_set1_ps(area);

  U64 fill_pixels_begin = __rdtsc();
  for (S64 y = min_y; y < max_y; y++) {
    F32x8 Cx0 = _mm256_set1_ps(Cy0);
    F32x8 Cx1 = _mm256_set1_ps(Cy1);
    F32x8 Cx2 = _mm256_set1_ps(Cy2);

    for (S64 x8 = min_x; x8 < max_x; x8+=8) {
      {
        F32x8 i0 = _mm256_set1_ps(I(Cx0, 7));
        F32x8 i1 = _mm256_add_ps(i0, Dy10.simd);
        Cx0 = {i1};

        F32x8 i2 = _mm256_set1_ps(I(Cx1, 7));
        F32x8 i3 = _mm256_add_ps(i2, Dy21.simd);
        Cx1 = {i3};

        F32x8 i4 = _mm256_set1_ps(I(Cx2, 7));
        F32x8 i5 = _mm256_add_ps(i4, Dy02.simd);
        Cx2 = {i5};
      }


      F32x8 should_fill;
        F32x8 i11 = _mm256_set1_ps(x8);
        F32x8 i12 = _mm256_add_ps(i11, var07);
        F32x8 i13 = _mm256_cmp_ps(i12, var_max_x, _CMP_LT_OQ);

        F32x8 i6  = _mm256_cmp_ps(Cx0, var0, _CMP_GE_OQ);
        F32x8 i7  = _mm256_cmp_ps(Cx1, var0, _CMP_GE_OQ);
        F32x8 i8  = _mm256_cmp_ps(Cx2, var0, _CMP_GE_OQ);
        F32x8 i9  = _mm256_and_ps(i6, i7);
        F32x8 i10 = _mm256_and_ps(i9, i8);
        should_fill = _mm256_and_ps(i13, i10);

      F32x8 w0 = _mm256_div_ps(Cx1, area8);
      F32x8 w1 = _mm256_div_ps(Cx2, area8);
      F32x8 w2 = _mm256_div_ps(Cx0, area8);

      // @Note: We could do: interpolated_w = 1.f / interpolated_w to get proper depth
      // but why waste an instruction, the smaller the depth value the farther the object
      F32x8 interpolated_w;
        F32x8 i14 = _mm256_mul_ps(one_over_p0w, w0); //
        F32x8 i15 = _mm256_mul_ps(one_over_p1w, w1);
        F32x8 i16 = _mm256_mul_ps(one_over_p2w, w2);
        F32x8 i17 = _mm256_add_ps(i14, i15);
        F32x8 i18 = _mm256_add_ps(i16, i17);
        interpolated_w = {i18};

      F32 *depth_pointer = (depth_buffer + (x8 + y * dst->x));
      F32x8 depth = _mm256_loadu_ps((float *)depth_pointer);

      //
      F32x8 should_fill_term = _mm256_cmp_ps(depth, interpolated_w, _CMP_LT_OQ);
      should_fill = _mm256_and_ps(should_fill, should_fill_term);

      F32x8 invw0 = _mm256_div_ps(w0, var_p0w);
      F32x8 invw1 = _mm256_div_ps(w1, var_p1w);
      F32x8 invw2 = _mm256_div_ps(w2, var_p2w);

      F32x8 u_term0 = _mm256_mul_ps(var_tex0x, invw0);
      F32x8 u_term1 = _mm256_mul_ps(var_tex1x, invw1);
      F32x8 u_term2 = _mm256_mul_ps(var_tex2x, invw2);
      F32x8 u_term3 = _mm256_add_ps(u_term0, u_term1);
      F32x8 u0      = _mm256_add_ps(u_term2, u_term3);

      F32x8 v_term0 = _mm256_mul_ps(var_tex0y, invw0);
      F32x8 v_term1 = _mm256_mul_ps(var_tex1y, invw1);
      F32x8 v_term2 = _mm256_mul_ps(var_tex2y, invw2);
      F32x8 v_term3 = _mm256_add_ps(v_term0, v_term1);
      F32x8 v0      = _mm256_add_ps(v_term2, v_term3);

      F32x8 u1 = _mm256_div_ps(u0, interpolated_w);
      F32x8 v1 = _mm256_div_ps(v0, interpolated_w);

      F32x8 u_floored = _mm256_floor_ps(u1);
      F32x8 v_floored = _mm256_floor_ps(v1);
      F32x8 u2 = _mm256_sub_ps(u1, u_floored);
      F32x8 v2 = _mm256_sub_ps(v1, v_floored);
      F32x8 u3 = _mm256_mul_ps(u2, var_src_x_minus_one);
      F32x8 v3 = _mm256_mul_ps(v2, var_src_y_minus_one);

      F32x8 ui = _mm256_cvtps_epi32(u3);
      F32x8 vi = _mm256_cvtps_epi32(v3);

      // Origin UV (0,0) is in bottom left
      _mm256_maskstore_epi32((int *)depth_pointer, should_fill, interpolated_w);

      S32x8 indices0 = _mm256_set1_epi32(src->y - 1);
      S32x8 indices1 = _mm256_sub_epi32(indices0, vi);
      S32x8 indices3 = _mm256_mullo_epi32(_mm256_set1_epi32(src->x), indices1);
      S32x8 indices  = _mm256_add_epi32(indices3, ui);




      //
      // Fetch and calculate texel values
      //
      Vec8I pixel;
      if(I(should_fill, 0)) pixel.e[0] = src->pixels[Is(indices, 0)];
      if(I(should_fill, 1)) pixel.e[1] = src->pixels[Is(indices, 1)];
      if(I(should_fill, 2)) pixel.e[2] = src->pixels[Is(indices, 2)];
      if(I(should_fill, 3)) pixel.e[3] = src->pixels[Is(indices, 3)];
      if(I(should_fill, 4)) pixel.e[4] = src->pixels[Is(indices, 4)];
      if(I(should_fill, 5)) pixel.e[5] = src->pixels[Is(indices, 5)];
      if(I(should_fill, 6)) pixel.e[6] = src->pixels[Is(indices, 6)];
      if(I(should_fill, 7)) pixel.e[7] = src->pixels[Is(indices, 7)];

      Vec8I texel_i_a = pixel & vec8i(0xff000000);
      Vec8I texel_i_b = pixel & vec8i(0x00ff0000);
      Vec8I texel_i_g = pixel & vec8i(0x0000ff00);
      Vec8I texel_i_r = pixel & vec8i(0x000000ff);

      // Alpha is done this way because signed integer shift is weird
      // When sign bit is set it sets all bits that we shift the sign through
      // So first we shift
      texel_i_a = (texel_i_a >> 24);
      texel_i_a = texel_i_a & vec8i(0x000000ff);
      texel_i_b = (texel_i_b >> 16);
      texel_i_g = (texel_i_g >> 8 );
      texel_i_r = (texel_i_r >> 0 );

      Vec8 texel_a = convert_vec8i_to_vec8(texel_i_a);
      Vec8 texel_b = convert_vec8i_to_vec8(texel_i_b);
      Vec8 texel_g = convert_vec8i_to_vec8(texel_i_g);
      Vec8 texel_r = convert_vec8i_to_vec8(texel_i_r);

      Vec8 v255 = vec8(255.f);
      texel_a = texel_a / v255;
      texel_b = texel_b / v255;
      texel_g = texel_g / v255;
      texel_r = texel_r / v255;

      texel_r = texel_r * texel_r;
      texel_g = texel_g * texel_g;
      texel_b = texel_b * texel_b;

      //
      // Fetch and calculate dst pixels
      //
      U32 *dst_memory = destination + x8;
      Vec8I dst_pixel = {_mm256_maskload_epi32((const int *)dst_memory, should_fill)};

      Vec8I dst_i_a = dst_pixel & vec8i(0xff000000);
      Vec8I dst_i_b = dst_pixel & vec8i(0x00ff0000);
      Vec8I dst_i_g = dst_pixel & vec8i(0x0000ff00);
      Vec8I dst_i_r = dst_pixel & vec8i(0x000000ff);

      dst_i_a = dst_i_a >> 24;
      dst_i_a = dst_i_a &  vec8i(0x000000ff);
      dst_i_b = dst_i_b >> 16 ;
      dst_i_g = dst_i_g >> 8;

      Vec8 dst_a = convert_vec8i_to_vec8(dst_i_a);
      Vec8 dst_b = convert_vec8i_to_vec8(dst_i_b);
      Vec8 dst_g = convert_vec8i_to_vec8(dst_i_g);
      Vec8 dst_r = convert_vec8i_to_vec8(dst_i_r);

      dst_a.simd = _mm256_div_ps(dst_a.simd, var255);
      dst_b.simd = _mm256_div_ps(dst_b.simd, var255);
      dst_g.simd = _mm256_div_ps(dst_g.simd, var255);
      dst_r.simd = _mm256_div_ps(dst_r.simd, var255);

      dst_r *= dst_r;
      dst_g *= dst_g;
      dst_b *= dst_b;

      // Premultiplied alpha
      {
        dst_r = texel_r + ((var1-texel_a) * dst_r);
        dst_g = texel_g + ((var1-texel_a) * dst_g);
        dst_b = texel_b + ((var1-texel_a) * dst_b);
        dst_a = texel_a + dst_a - texel_a*dst_a;
      }

      // Almost linear to srgb
      {
        dst_r.simd = {_mm256_sqrt_ps(dst_r.simd)};
        dst_g.simd = {_mm256_sqrt_ps(dst_g.simd)};
        dst_b.simd = {_mm256_sqrt_ps(dst_b.simd)};
      }

      Vec8I result;
      for(S64 i = 0; i < 8; i++){
        if (I(should_fill, i)){
            U8 red     = (U8)(dst_r[i] * 255);
            U8 green   = (U8)(dst_g[i] * 255);
            U8 blue    = (U8)(dst_b[i] * 255);
            U8 alpha   = (U8)(dst_a[i] * 255);
            result.e[i] = (U32)(alpha << 24 | blue << 16 | green << 8 | red << 0);
        }
      }

      _mm256_maskstore_epi32((int *)dst_memory, should_fill, result.simd);

    }
    Cy0 -= dx10;
    Cy1 -= dx21;
    Cy2 -= dx02;
    destination += dst->x;
  }
  U64 end_time = __rdtsc();

  filled_pixel_total_time += end_time - fill_pixels_begin;
  filled_pixel_count      += (max_x - min_x)*(max_y - min_y);
}

function
void draw_mesh(Render *r, String scene_name, Obj_Material *materials, Obj_Mesh *mesh, Vec3 *vertices, Vec2 *tex_coords, Vec3 *normals) {
  // ZoneNamedN(m, "draw_all_meshes", true);
  PROFILE_SCOPE(draw_all_meshes);
  for (int i = 0; i < mesh->indices.len; i++) {
    PROFILE_SCOPE(draw_set_of_mesh_indices);
    // ZoneNamedN(m, "draw_single_mesh", true);
    Obj_Index *index = mesh->indices.data + i;
    Bitmap *image = &r->img;
    if(index->material_id != -1) {
      Obj_Material *material = materials + index->material_id;
      // @Todo: No size info from OBJ things, this stuff needs a bit of refactor
      //        Need to figure out how to accomodate multiple possible formats of input etc.
      if(material->texture_ambient.pixels) {
        image = &material->texture_ambient;
      }
    }

    Vertex vert[] = {
      {
        vertices[index->vertex[0] - 1],
        tex_coords[index->tex[0] - 1],
        normals[index->normal[0] - 1],
      },
      {
        vertices[index->vertex[1] - 1],
        tex_coords[index->tex[1] - 1],
        normals[index->normal[1] - 1],
      },
      {
        vertices[index->vertex[2] - 1],
        tex_coords[index->tex[2] - 1],
        normals[index->normal[2] - 1],
      },
    };

    //@Note: Transform
    for (int j = 0; j < 3; j++) {
      vert[j].pos = r->transform * vert[j].pos;
    }


    Vec3 p0_to_camera = r->camera_pos - vert[0].pos;
    Vec3 p0_to_p1 = vert[1].pos - vert[0].pos;
    Vec3 p0_to_p2 = vert[2].pos - vert[0].pos;
    Vec3 normal = normalize(cross(p0_to_p1, p0_to_p2));
    Vec3 light_direction =  mat4_rotation_x(light_rotation) * vec3(0, 0, 1);

    if (dot(normal, p0_to_camera) > 0) { //@Note: Backface culling
      /// ## Clipping
      ///
      /// There are 3 clipping stages, 2 clipping stages in 3D space against zfar and znear and 1 clipping
      /// stage in 2D against left, bottom, right, top(2D image bounds).
      ///
      /// First the triangles get clipped against the zfar plane,
      /// if a triangle has even one vertex outside the clipping region, the entire triangle gets cut.
      /// So far I didn't have problems with that. It simplifies the computations and splitting triangles
      /// on zfar seems like a waste of power.
      ///
      /// The second clipping stage is znear plane. Triangles get fully and nicely clipped against znear.
      /// Every time a triangle gets partially outside the clipping region it gets cut to the znear and
      /// either one or two new triangles get derived from the old one.
      ///
      /// Last clipping stage is performed in the 2D image space. Every triangle has a corresponding AABB
      /// box. In this box every pixel gets tested to see if it's in the triangle. In this clipping stage
      /// the box is clipped to the image metrics - 0, 0, width, height.
      ///
      ///
      // @Note: Zfar
      B32 vertex_is_outside = false;
      Vec3 zfar_normal = vec3(0, 0, -1);
      Vec3 zfar_pos = vec3(0, 0, zfar_value);
      for (S32 j = 0; j < 3; j++) {
        // @Note: Camera
        vert[j].pos = r->camera * vert[j].pos;
        // @Note: Skip triangle if even one vertex gets outside the clipping plane
        if ((dot(zfar_normal, vert[j].pos - zfar_pos) < 0)) {
          vertex_is_outside = true;
          break;
        }
      }

      if (vertex_is_outside) {
        continue;
      }

      // @Note: Znear, clip triangles to the near clipping plane
      Vec3 znear_normal = vec3(0, 0, 1);
      Vec3 znear_pos = vec3(0, 0, 1.f);

      struct _Vertex {
        Vec4 pos;
        Vec2 tex;
        Vec3 norm;
      } in[4];
      S32 in_count = 0;

      Vertex *prev = vert + 2;
      Vertex *curr = vert;
      F32       prev_dot = dot(znear_normal, prev->pos - znear_pos);
      F32       curr_dot = 0;
      for (int j = 0; j < 3; j++) {
        curr_dot = dot(znear_normal, curr->pos - znear_pos);
        if (curr_dot * prev_dot < 0) {
          F32 t = prev_dot / (prev_dot - curr_dot);
          in[in_count].pos = vec4(lerp(prev->pos, curr->pos, t), 1);
          in[in_count].tex = lerp(prev->tex, curr->tex, t);
          in[in_count].norm = lerp(prev->norm, curr->norm, t);
          in_count += 1;
        }

        if (curr_dot > 0) {
          in[in_count].pos = vec4(vert[j].pos, 1);
          in[in_count].tex = vert[j].tex;
          in[in_count++].norm = vert[j].norm;
        }

        prev = curr++;
        prev_dot = curr_dot;
      }

      if (in_count == 0) {
        continue;
      }

      for(S64 j = 0; j < in_count; j++) {
        //@Note: Perspective
        in[j].pos = r->projection * in[j].pos;
        in[j].pos.x = in[j].pos.x / in[j].pos.w;
        in[j].pos.y = in[j].pos.y / in[j].pos.w;
        // in[j].pos.z = in[j].pos.z / in[j].pos.w;

        //@Note: To pixel space
        in[j].pos.x *= r->screen320.x / 2;
        in[j].pos.y *= r->screen320.y / 2;
        in[j].pos.x += r->screen320.x / 2;
        in[j].pos.y += r->screen320.y / 2;
      }


      draw_triangle_nearest(&r->screen320, r->depth320, image, light_direction, in[0].pos, in[1].pos, in[2].pos, in[0].tex, in[1].tex, in[2].tex, in[0].norm, in[1].norm, in[2].norm);
      if (in_count > 3) {
        draw_triangle_nearest(&r->screen320, r->depth320, image, light_direction, in[0].pos, in[2].pos, in[3].pos, in[0].tex, in[2].tex, in[3].tex, in[0].norm, in[2].norm, in[3].norm);
      }
    }
  }
}

#include "ui.cpp"
global F32 speed = 100.f;
global F32 rotation = 0;
global Obj *f22;
global Obj *sponza;
global Obj *obj;
global Render r = {};
global Scene scene = Scene_Sponza;

function
UI_SIGNAL_CALLBACK(scene_callback) {
  switch(scene) {
    case Scene_F22: {
      speed = 1;
      r.camera_pos = vec3(0,0,-2);
      obj = f22;
    } break;
    case Scene_Sponza: {
      speed = 100;
      r.camera_pos = vec3(-228,94.5,-107);
      r.camera_yaw = vec2(-1.25, 0.21);
      obj = sponza;
    } break;
    case Scene_Count:
    invalid_default_case;
  }
  scene = (Scene)(((int)scene + 1) % Scene_Count);
}

FILE *global_file;
function void
windows_log(Log_Kind kind, String string, char *file, int line){
  fprintf(global_file, "%s", string.str);
}

int
main(int argc, char **argv) {
  global_file = fopen("perfclocks.txt", "a");
  thread_ctx.log_proc = windows_log;
  fprintf(global_file, "\n---------------------");


  os.window_size.x = 1280;
  os.window_size.y = 720;
  os.window_resizable = 1;
  assert(os_init());
  Font font = os_load_font(os.perm_arena, 12*os.dpi_scale, "Arial", 0);

  f22 = load_obj_dump(os.perm_arena, "plane.bin"_s);
  sponza = load_obj_dump(os.perm_arena, "sponza.bin"_s);
   // Obj sponza_obj = load_obj(&os_process_heap, "assets/sponza/sponza.obj"_s);
  // sponza = &sponza_obj;
  scene_callback();

  int screen_x = 1280;
  int screen_y = 720;

  r.camera_pos = vec3(-228,94.5,-107);
  r.camera_yaw = vec2(-1.25, 0.21);
  r.screen320 = {(U32 *)arena_push_size(os.perm_arena, screen_x*screen_y*sizeof(U32)), screen_x, screen_y};
  r.depth320 = (F32 *)arena_push_size(os.perm_arena, sizeof(F32) * screen_x * screen_y);

  String frame_data = {};
  UISetup setup[] = {
    UI_SIGNAL("Change scene"_s, scene_callback),
    UI_LABEL(&frame_data),
    UI_LABEL(&os.text),
  };
  UI ui = ui_make(setup, buff_cap(setup));
  B32 ui_mouse_lock = true;

  while (os_game_loop()) {
    PROFILE_SCOPE(main_loop);
    if (ui_mouse_lock == false) {
      r.camera_yaw.x += os.delta_mouse_pos.x * 0.01f;
      r.camera_yaw.y -= os.delta_mouse_pos.y * 0.01f;
    }
    if (os.key[Key_Escape].pressed) os_quit();
    if (os.key[Key_O].down) light_rotation += 0.05f;
    if (os.key[Key_P].down) light_rotation -= 0.05f;
    if (os.key[Key_F2].pressed) {
      ui_mouse_lock = !ui_mouse_lock;
      os_show_cursor(!os.cursor_visible);
    }
    if (os.key[Key_A].down) r.camera_pos.x -= speed * (F32)os.delta_time;
    if (os.key[Key_D].down) r.camera_pos.x += speed * (F32)os.delta_time;
    if (os.key[Key_W].down) {
      r.camera_forward_velocity = r.camera_direction * speed * (F32)os.delta_time;
      r.camera_pos = r.camera_pos + r.camera_forward_velocity;
    }
    if (os.key[Key_S].down) {
      r.camera_forward_velocity = r.camera_direction * speed * (F32)os.delta_time;
      r.camera_pos = r.camera_pos - r.camera_forward_velocity;
    }
    if (os.key[Key_R].down) r.camera_pos.y += speed * (F32)os.delta_time;
    if (os.key[Key_F].down) r.camera_pos.y -= speed * (F32)os.delta_time;

    // Clear screen and depth buffer
    U32* p = r.screen320.pixels;
    for (int y = 0; y < r.screen320.y; y++) {
      for (int x = 0; x < r.screen320.x; x++) {
        *p++ = 0x33333333;
      }
    }

    F32* dp = r.depth320;
    for (int y = 0; y < r.screen320.y; y++) {
      for (int x = 0; x < r.screen320.x; x++) {
        *dp++ = -F32MAX;
      }
    }

    Mat4 camera_rotation = mat4_rotation_y(r.camera_yaw.x) * mat4_rotation_x(r.camera_yaw.y);
    r.camera_direction = (camera_rotation * vec4(0,0,1,1)).xyz;
    Vec3 target = r.camera_pos + r.camera_direction;
    r.camera = mat4_look_at(r.camera_pos, target, vec3(0, 1, 0));
    r.projection = mat4_perspective(60.f, (F32)os.screen->x, (F32)os.screen->y, 1.f, zfar_value);
    r.transform = mat4_rotation_z(rotation);
    r.transform = r.transform * mat4_rotation_y(rotation);
    for (int i = 0; i < obj->mesh.len; i++) {
      PROFILE_SCOPE(draw_all_meshes);
      Vec2* tex_coords = (Vec2*)obj->texture_coordinates.data;
      Vec3 *normals = (Vec3 *)obj->normals.data;
      Obj_Mesh *mesh = obj->mesh.data;
      Vec3* vertices = (Vec3 *)obj->vertices.data;
      draw_mesh(&r, obj->name, obj->materials.data, mesh+i, vertices, tex_coords, normals);
    }


    // @Note: Draw 320screen to OS screen
    U32* ptr = os.screen->pixels;
    for (int y = 0; y < os.screen->y; y++) {
      for (int x = 0; x < os.screen->x; x++) {
        F32 u = (F32)x / (F32)os.screen->x;
        F32 v = (F32)y / (F32)os.screen->y;
        int tx = (int)(u * r.screen320.x );
        int ty = (int)(v * r.screen320.y );
        *ptr++ = r.screen320.pixels[tx + ty * (r.screen320.x)];
      }
    }

    ui_end_frame(os.screen, &ui, &font);
    frame_data = string_fmt(os.frame_arena, "FPS:%f dt:%f frame:%u camera_pos: %f %f %f camera_yaw: %f %f", os.fps, os.delta_time, os.frame,
      r.camera_pos.x, r.camera_pos.y, r.camera_pos.z, r.camera_yaw.x, r.camera_yaw.y);


    // log_info("\nAvg_Time: %llu Time:%llu Count:%llu", filled_pixel_total_time/filled_pixel_count, filled_pixel_total_time, filled_pixel_count);
    for(int i = 0; i < ProfileScopeName_Count; i++){
      auto *scope = &profile_scopes[i];
      if(scope->i == 0) continue;

      U64 total = 0;
      for(int i = 0; i < scope->i; i++){
        total += scope->samples[i];
      }

      log_info("\n%s :: Total: %llu Hits: %llu, Avg: %llu", profile_scope_names[i], total, (U64)scope->i, total / scope->i);
      scope->i = 0;
    }
  }
}

/////////////////////////////////////////////////////////////////////////////////////
/// ### Resources that helped me build the rasterizer (Might be helpful to you too):
///
/// * Algorithm I used for triangle rasterization by Juan Pineda is described in paper called "A Parallel Algorithm for Polygon Rasterization"
/// * Casey Muratori's series on making a game from scratch(including a 2D software rasterizer(episode ~82) and 3d gpu renderer): https://hero.handmade.network/episode/code#
/// * Fabian Giessen's "Optimizing Software Occlusion Culling": https://fgiesen.wordpress.com/2013/02/17/optimizing-sw-occlusion-culling-index/
/// * Fabian Giessen's optimized software renderer: https://github.com/rygorous/intel_occlusion_cull/tree/blog/SoftwareOcclusionCulling
/// * Fabian Giessen's javascript triangle rasterizer: https://gist.github.com/rygorous/2486101
/// * Fabian Giessen's C++ triangle rasterizer: https://github.com/rygorous/trirast/blob/master/main.cpp
/// * Joy's Kenneth lectures about computer graphics: https://www.youtube.com/playlist?list=PL_w_qWAQZtAZhtzPI5pkAtcUVgmzdAP8g
/// * Joy's Kenneth article on clipping: https://import.cdn.thinkific.com/167815/JoyKennethClipping-200905-175314.pdf
/// * A bunch of helpful notes and links to resources: https://nlguillemot.wordpress.com/2016/07/10/rasterizer-notes/
/// * Very nice paid course on making a software rasterizer using a scanline method: https://pikuma.com/courses/learn-3d-computer-graphics-programming
/// * Reference for obj loader: https://github.com/tinyobjloader/tinyobjloader/blob/master/tiny_obj_loader.h
///
/// ### To read
///
/// * http://ce-publications.et.tudelft.nl/publications/1362_hardware_algorithms_for_tilebased_realtime_rendering.pdf
