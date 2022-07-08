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
///   - [x] Inline edge function
///   - [x] Expand edge functions to more optimized version
///   - [ ] Test 4x2 bitmap layout?
///   - [ ] Edge function to integer
///   - [ ] Use integer bit operations to figure out if plus. (edge0|edge1|edge2)>=0
///   - [x] SIMD
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
/// - [x] Asset processor as second program

// #include "obj_dump.cpp"
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

#define F32x8 __m256
#define S32x8 __m256i

U64 filled_pixel_count;
U64 filled_pixel_total_time;
#include "optimization_log.cpp"

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

  F32x8 var255 = _mm256_set1_ps(255);
  F32x8 var0 = _mm256_set1_ps(0);
  F32x8 var1 = _mm256_set1_ps(1);
  F32x8 var_max_x = _mm256_set1_ps(max_x);
  F32x8 var07 = _mm256_set_ps(7,6,5,4,3,2,1,0);
  F32x8 inv255 = _mm256_div_ps(var1, var255);

  F32x8 var_src_x_minus_one = _mm256_set1_ps(src->x-1);
  F32x8 var_src_y_minus_one = _mm256_set1_ps(src->y-1);
  S32x8 var_src_y_minus_one_int = _mm256_set1_epi32(src->y-1);
  S32x8 var_src_x_int = _mm256_set1_epi32(src->x);

  S32x8 var_0xff000000 = _mm256_set1_epi32(0xff000000);
  S32x8 var_0x00ff0000 = _mm256_set1_epi32(0x00ff0000);
  S32x8 var_0x0000ff00 = _mm256_set1_epi32(0x0000ff00);
  S32x8 var_0x000000ff = _mm256_set1_epi32(0x000000ff);

  F32x8 var_tex0x = _mm256_set1_ps(tex0.x);
  F32x8 var_tex1x = _mm256_set1_ps(tex1.x);
  F32x8 var_tex2x = _mm256_set1_ps(tex2.x);
  F32x8 var_tex0y = _mm256_set1_ps(tex0.y);
  F32x8 var_tex1y = _mm256_set1_ps(tex1.y);
  F32x8 var_tex2y = _mm256_set1_ps(tex2.y);

  F32x8 inv_p0w = _mm256_div_ps(var1, _mm256_set1_ps(p0.w));
  F32x8 inv_p1w = _mm256_div_ps(var1, _mm256_set1_ps(p1.w));
  F32x8 inv_p2w = _mm256_div_ps(var1, _mm256_set1_ps(p2.w));
  F32x8 one_over_p0w = _mm256_set1_ps(1.f / p0.w);
  F32x8 one_over_p1w = _mm256_set1_ps(1.f / p1.w);
  F32x8 one_over_p2w = _mm256_set1_ps(1.f / p2.w);

  U32 *destination = dst->pixels + dst->x*min_y;
  F32 area = (p1.y - p0.y) * (p2.x - p0.x) - (p1.x - p0.x) * (p2.y - p0.y);
  F32x8 inv_area8 = _mm256_div_ps(var1, _mm256_set1_ps(area));

  F32x8 _dy10 = _mm256_set1_ps(dy10);
  F32x8 _dx10 = _mm256_set1_ps(dx10);
  F32x8 _dy21 = _mm256_set1_ps(dy21);
  F32x8 _dx21 = _mm256_set1_ps(dx21);
  F32x8 _dy02 = _mm256_set1_ps(dy02);
  F32x8 _dx02 = _mm256_set1_ps(dx02);
  F32x8 p0_x = _mm256_set1_ps(p0.x);
  F32x8 p0_y = _mm256_set1_ps(p0.y);
  F32x8 p1_x = _mm256_set1_ps(p1.x);
  F32x8 p1_y = _mm256_set1_ps(p1.y);
  F32x8 p2_x = _mm256_set1_ps(p2.x);
  F32x8 p2_y = _mm256_set1_ps(p2.y);

  U64 fill_pixels_begin = __rdtsc();
  for (S64 y = min_y; y < max_y; y++) {

    F32x8 Y = _mm256_set1_ps(y);
    for (S64 x8 = min_x; x8 < max_x; x8+=8) {
      F32x8 X = _mm256_add_ps(_mm256_set1_ps(x8), var07);

      // F32x8 Cx0 = (p1.y - p0.y) * (p.x - p0.x) - (p1.x - p0.x) * (p.y - p0.y);
      F32x8 px_minus_0x = _mm256_sub_ps(X, p0_x);
      F32x8 py_minus_0y = _mm256_sub_ps(Y, p0_y);
      F32x8 left0 = _mm256_mul_ps(_dy10, px_minus_0x);
      F32x8 right0 = _mm256_mul_ps(_dx10, py_minus_0y);
      F32x8 Cx0 = _mm256_sub_ps(left0,right0);

      // F32 result = (p2.y - p1.y) * (p.x - p1.x) - (p2.x - p1.x) * (p.y - p1.y);
      F32x8 px_minus_1x = _mm256_sub_ps(X, p1_x);
      F32x8 py_minus_1y = _mm256_sub_ps(Y, p1_y);
      F32x8 left1 = _mm256_mul_ps(_dy21, px_minus_1x);
      F32x8 right1 = _mm256_mul_ps(_dx21, py_minus_1y);
      F32x8 Cx1 = _mm256_sub_ps(left1,right1);

      // F32 result = (p0.y - p2.y) * (p.x - p2.x) - (p0.x - p2.x) * (p.y - p2.y);
      F32x8 px_minus_2x = _mm256_sub_ps(X, p2_x);
      F32x8 py_minus_2y = _mm256_sub_ps(Y, p2_y);
      F32x8 left2 = _mm256_mul_ps(_dy02, px_minus_2x);
      F32x8 right2 = _mm256_mul_ps(_dx02, py_minus_2y);
      F32x8 Cx2 = _mm256_sub_ps(left2,right2);


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

      F32x8 w0 = _mm256_mul_ps(Cx1, inv_area8);
      F32x8 w1 = _mm256_mul_ps(Cx2, inv_area8);
      F32x8 w2 = _mm256_mul_ps(Cx0, inv_area8);

      // @Todo: Turn this into 1 / interpolated_w, turns out in theory it should be
      // more performant but couldn't make it work

      // @Old_Note: We could do: interpolated_w = 1.f / interpolated_w to get proper depth
      // but why waste an instruction, the smaller the depth value the farther the object
      F32x8 interpolated_w;
        F32x8 i14 = _mm256_mul_ps(one_over_p0w, w0); //
        F32x8 i15 = _mm256_mul_ps(one_over_p1w, w1);
        F32x8 i16 = _mm256_mul_ps(one_over_p2w, w2);
        F32x8 i17 = _mm256_add_ps(i14, i15);
        F32x8 i18 = _mm256_add_ps(i16, i17);
        interpolated_w = {i18};

      F32 *depth_pointer = (depth_buffer + (x8 + y * dst->x));
      F32x8 depth = _mm256_loadu_ps(depth_pointer);

      F32x8 should_fill_term = _mm256_cmp_ps(depth, interpolated_w, _CMP_LT_OQ);
      should_fill = _mm256_and_ps(should_fill, should_fill_term);

      // If all pixels are not going to get drawn then opt out
      F32x8 compare_with_zero = _mm256_cmpeq_epi32(should_fill, var0);
      int mask = _mm256_movemask_epi8(compare_with_zero);
      if(mask == 0xffffffff) {
        continue;
      }

      F32x8 invw0 = _mm256_mul_ps(w0, inv_p0w);
      F32x8 invw1 = _mm256_mul_ps(w1, inv_p1w);
      F32x8 invw2 = _mm256_mul_ps(w2, inv_p2w);

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

      S32x8 indices1 = _mm256_sub_epi32(var_src_y_minus_one_int, vi);
      S32x8 indices3 = _mm256_mullo_epi32(var_src_x_int, indices1);
      S32x8 indices  = _mm256_add_epi32(indices3, ui);


      //
      // Fetch and calculate texel values
      //
      indices = _mm256_and_si256(indices, should_fill);
      S32x8 pixel = _mm256_set_epi32(
        src->pixels[_mm256_extract_epi32(indices, 7)],
        src->pixels[_mm256_extract_epi32(indices, 6)],
        src->pixels[_mm256_extract_epi32(indices, 5)],
        src->pixels[_mm256_extract_epi32(indices, 4)],
        src->pixels[_mm256_extract_epi32(indices, 3)],
        src->pixels[_mm256_extract_epi32(indices, 2)],
        src->pixels[_mm256_extract_epi32(indices, 1)],
        src->pixels[_mm256_extract_epi32(indices, 0)]
      );

      S32x8 texel_i_a = _mm256_and_si256(pixel, var_0xff000000);
      S32x8 texel_i_b = _mm256_and_si256(pixel, var_0x00ff0000);
      S32x8 texel_i_g = _mm256_and_si256(pixel, var_0x0000ff00);
      S32x8 texel_i_r = _mm256_and_si256(pixel, var_0x000000ff);

      // Alpha is done this way because signed integer shift is weird
      // When sign bit is set it sets all bits that we shift the sign through
      // So first we shift
      texel_i_a = _mm256_srli_epi32(texel_i_a, 24);
      texel_i_b = _mm256_srli_epi32(texel_i_b, 16);
      texel_i_g = _mm256_srli_epi32(texel_i_g, 8 );
      texel_i_r = _mm256_srli_epi32(texel_i_r, 0 );

      F32x8 texel_a0 = _mm256_cvtepi32_ps(texel_i_a);
      F32x8 texel_b0 = _mm256_cvtepi32_ps(texel_i_b);
      F32x8 texel_g0 = _mm256_cvtepi32_ps(texel_i_g);
      F32x8 texel_r0 = _mm256_cvtepi32_ps(texel_i_r);

      F32x8 texel_b1 = _mm256_mul_ps(texel_b0, inv255);
      F32x8 texel_g1 = _mm256_mul_ps(texel_g0, inv255);
      F32x8 texel_r1 = _mm256_mul_ps(texel_r0, inv255);
      F32x8 texel_a1 = _mm256_mul_ps(texel_a0, inv255);

      texel_r1 = _mm256_mul_ps(texel_r1, texel_r1);
      texel_g1 = _mm256_mul_ps(texel_g1, texel_g1);
      texel_b1 = _mm256_mul_ps(texel_b1, texel_b1);

      //
      // Fetch and calculate dst pixels
      //
      U32 *dst_memory = destination + x8;
      S32x8 dst_pixel = _mm256_maskload_epi32((const int *)dst_memory, should_fill);

      S32x8 dst_i_a0 = _mm256_and_si256(dst_pixel, var_0xff000000);
      S32x8 dst_i_b0 = _mm256_and_si256(dst_pixel, var_0x00ff0000);
      S32x8 dst_i_g0 = _mm256_and_si256(dst_pixel, var_0x0000ff00);
      S32x8 dst_i_r0 = _mm256_and_si256(dst_pixel, var_0x000000ff);

      S32x8 dst_i_a1 = _mm256_srai_epi32(dst_i_a0, 24);
      dst_i_a1 = _mm256_and_si256(dst_i_a1, var_0x000000ff);
      S32x8 dst_i_b1 = _mm256_srai_epi32(dst_i_b0, 16);
      S32x8 dst_i_g1 = _mm256_srai_epi32(dst_i_g0, 8);
      S32x8 dst_i_r1 = dst_i_r0;

      F32x8 dst_a = _mm256_cvtepi32_ps(dst_i_a1);
      F32x8 dst_b = _mm256_cvtepi32_ps(dst_i_b1);
      F32x8 dst_g = _mm256_cvtepi32_ps(dst_i_g1);
      F32x8 dst_r = _mm256_cvtepi32_ps(dst_i_r1);

      dst_a = _mm256_mul_ps(dst_a, inv255);
      dst_b = _mm256_mul_ps(dst_b, inv255);
      dst_g = _mm256_mul_ps(dst_g, inv255);
      dst_r = _mm256_mul_ps(dst_r, inv255);

      dst_r = _mm256_mul_ps(dst_r, dst_r);
      dst_g = _mm256_mul_ps(dst_g, dst_g);
      dst_b = _mm256_mul_ps(dst_b, dst_b);

      // Premultiplied alpha
      {
        dst_r = _mm256_add_ps(texel_r1, _mm256_mul_ps(_mm256_sub_ps(var1,texel_a1), dst_r));
        dst_g = _mm256_add_ps(texel_g1, _mm256_mul_ps(_mm256_sub_ps(var1,texel_a1), dst_g));
        dst_b = _mm256_add_ps(texel_b1, _mm256_mul_ps(_mm256_sub_ps(var1,texel_a1), dst_b));
        dst_a = _mm256_sub_ps(_mm256_add_ps(texel_a1, dst_a), _mm256_mul_ps(texel_a1,dst_a));
      }

      // Almost linear to srgb
      {
        dst_r = _mm256_sqrt_ps(dst_r);
        dst_g = _mm256_sqrt_ps(dst_g);
        dst_b = _mm256_sqrt_ps(dst_b);
      }

      // Convert to integer format
      dst_r = _mm256_mul_ps(dst_r, var255);
      dst_g = _mm256_mul_ps(dst_g, var255);
      dst_b = _mm256_mul_ps(dst_b, var255);
      dst_a = _mm256_mul_ps(dst_a, var255);

      S32x8 dst_r_int = _mm256_cvtps_epi32(dst_r);
      S32x8 dst_g_int = _mm256_cvtps_epi32(dst_g);
      S32x8 dst_b_int = _mm256_cvtps_epi32(dst_b);
      S32x8 dst_a_int = _mm256_cvtps_epi32(dst_a);

      S32x8 dst_int_a_shifted = _mm256_slli_epi32(dst_a_int, 24);
      S32x8 dst_int_b_shifted = _mm256_slli_epi32(dst_b_int, 16);
      S32x8 dst_int_g_shifted = _mm256_slli_epi32(dst_g_int, 8);
      S32x8 dst_int_r_shifted = dst_r_int;

      S32x8 packed_abgr0 = _mm256_or_si256(dst_int_a_shifted, dst_int_b_shifted);
      S32x8 packed_abgr1 = _mm256_or_si256(dst_int_r_shifted, dst_int_g_shifted);
      S32x8 packed_abgr2 = _mm256_or_si256(packed_abgr1, packed_abgr0);

      _mm256_maskstore_epi32((int *)dst_memory, should_fill, packed_abgr2);
    }
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

const int ARRAY_LIST_DEFAULT_CAP = 32;
const int ARRAY_DEFAULT_ALLOCATION_MUL = 2;
template<class T> struct Array_List;
template<class T> void array_add_free_node(Arena *arena, Array_List<T> *array, int size);

template<class T>
struct Array_Node{
  Array_Node<T> *next;
  Array_Node<T> *prev;
  int cap, len;
  T data[];
};

template<class T>
struct Array_List{
  // int first_block_size;
  int allocation_multiplier = 0;
  Array_Node<T> *first = 0;
  Array_Node<T> *last  = 0;
  Array_Node<T> *first_free = 0;

  Array_List() = default;
  // Array_List(Arena *arena, int size){ array_add_free_node(arena, this, size); }
};

template<class T>
struct Array_List_Iter{
  T *item;
  int index;
  Array_Node<T> *node;
  int node_index;
};

template<class T>
void iter_next(Array_List_Iter<T> *iter){
  if(iter->node_index + 1 >= iter->node->len){
    iter->node = iter->node->next;
    iter->node_index = -1;
    iter->item = 0;
  }

  if(iter->node){
    iter->node_index += 1;
    iter->index += 1;
    iter->item = iter->node->data + iter->node_index;
  }
}

template<class T>
Array_List_Iter<T> iter_make(Array_List<T> *array){
  Array_List_Iter<T> result = {};
  result.node = array->first;
  result.index = result.node_index = -1;
  iter_next(&result);
  return result;
}

template<class T>
void array_remove_from_free_list(Array_List<T> *array, Array_Node<T> *it){
  if(it->prev) it->prev->next = it->next;
  if(it->next) it->next->prev = it->prev;
  if(array->first_free == it) array->first_free = array->first_free->next;
}

template<class T>
Array_Node<T> *array_allocate_node(Arena *arena, int size){
  auto node = (Array_Node<T> *)arena_push_size(arena, sizeof(Array_Node<T>) + size*sizeof(T));
  node->cap = size;
  node->len = 0;
  return node;
}

template<class T>
void array_add_free_node(Arena *arena, Array_List<T> *array, int size){
  Array_Node<T> *node = array_allocate_node<T>(arena, size);
  DLLFreeListAdd(array->first_free, node);
}

template<class T>
void make_sure_there_is_room_for_item_count(Arena *arena, Array_List<T> *array, int item_count){
  if(array->last == 0 || array->last->len + item_count > array->last->cap){
    // Not enough space we need to get a new block

    if(!array->allocation_multiplier) array->allocation_multiplier = ARRAY_DEFAULT_ALLOCATION_MUL;
    Array_Node<T> *node = 0;

    // Iterate the free list to check if we have a block of required size there
    for(Array_Node<T> *it = array->first_free; it; it=it->next){
      if(it->cap >= item_count){
        array_remove_from_free_list(array, it);
        node = it;
        node->len = 0;
        break;
      }
    }

    // We don't have a block on the free list need to allocate
    if(!node){
      assert(array->allocation_multiplier);
      int block_cap = array->last ? array->last->cap : ARRAY_LIST_DEFAULT_CAP / array->allocation_multiplier;
      block_cap *= array->allocation_multiplier;
      node = array_allocate_node<T>(arena, block_cap);
    }

    assert(node);
    DLLQueuePushLast(array->first, array->last, node);
  }
}

template<class T>
T *array_get(Array_List<T> *array, int index){
  int i = 0;
  for(Array_Node<T> *it = array->first; it; it=it->next){
    int lookup_i = index - i;
    if(lookup_i < it->len) return it->data + lookup_i;
    i += it->cap;
  }
  return 0;
}

template<class T>
void array_add(Arena *arena, Array_List<T> *array, T item){
  make_sure_there_is_room_for_item_count(arena, array, 1);
  array->last->data[array->last->len++] = item;
}

template<class T>
void array_free_node(Array_List<T> *array, Array_Node<T> *node){

#if 1
  // Make sure it's actually in array list
  B32 found = false;
  for(Array_Node<T> *it = array->first; it; it=it->next){
    if(it == node){
      found = true;
      break;
    }
  }
  assert(found);
#endif


  // Remove from array list
  if(array->first == array->last){
    assert(node == array->first);
    array->first = array->last = 0;
  }
  else if(array->last == node){
    array->last = array->last->prev;
    array->last->next = 0;
  }
  else if(array->first == node){
    array->first = array->first->next;
    array->first->prev = 0;
  }
  else{
    node->prev->next = node->next;
    node->next->prev = node->prev;
  }

  DLLFreeListAdd(array->first_free, node);
}

template<class T>
void array_free_all_nodes(Array_List<T> *array){
  assert(!array->last->next);
  assert(!array->first->prev);
  array->last->next = array->first_free;
  if(array->first_free) array->first_free->prev = array->last;
  array->first_free = array->first;
  array->last = array->first = 0;
}

void array_print(Array_List<int> *array){
  log_info("Nodes: ");
  for(Array_Node<int> *it = array->first; it; it=it->next){
    log_info("%d", it->cap);
    if(it->next) log_info("->");
  }

  log_info("\nFree: ");
  for(Array_Node<int> *it = array->first_free; it; it=it->next){
    log_info("%d", it->cap);
    if(it->next) log_info("->");
  }

  log_info("\nNodes_Reverse: ");
  for(Array_Node<int> *it = array->last; it; it=it->prev){
    log_info("%d", it->cap);
    if(it->prev) log_info("<-");
  }
}

FILE *global_file;
function void
windows_log(Log_Kind kind, String string, char *file, int line){
  // fprintf(global_file, "%s", string.str);
  OutputDebugStringA((const char *)string.str);
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
  Scratch scratch;
  Array_List<int> array;
  for(int i = 0; i < 512; i++){
    array_add(scratch, &array, i);
  }

  for(Array_List_Iter<int> i = iter_make(&array); i.item; iter_next(&i)){
    assert(i.index == *i.item);
  }
  assert(*array_get(&array, 22) == 22);
  assert(*array_get(&array, 65) == 65);
  assert(*array_get(&array, 200) == 200);

  array_free_node(&array, array.last);
  // array_free_node(&array, array.last);
  // array_free_node(&array, array.last);

  for(int i = 0; i < 33; i++){
    array_add(scratch, &array, i);
  }

  array_free_all_nodes(&array);
  array_print(&array);


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
