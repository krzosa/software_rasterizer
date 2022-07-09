#define I(x,i) (((F32 *)&x)[i])
#define Is(x,i) (((S32 *)&x)[i])

function
void draw_triangle_nearest_a(Bitmap* dst, F32 *depth_buffer, Bitmap *src, Vec3 light_direction,
                           Vec4 p0,   Vec4 p1,   Vec4 p2,
                           Vec2 tex0, Vec2 tex1, Vec2 tex2,
                           Vec3 norm0, Vec3 norm1, Vec3 norm2) {
  if(src->pixels == 0) return;
  U64 fill_pixels_begin = __rdtsc();

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


  U32 *destination = dst->pixels + dst->x*min_y;
  F32 area = (p1.y - p0.y) * (p2.x - p0.x) - (p1.x - p0.x) * (p2.y - p0.y);
  for (S64 y = min_y; y < max_y; y++) {
    for (S64 x = min_x; x < max_x; x++) {
      F32 Cx0 = edge_function(p0, p1, { (F32)x,(F32)y });
      F32 Cx1 = edge_function(p1, p2, { (F32)x,(F32)y });
      F32 Cx2 = edge_function(p2, p0, { (F32)x,(F32)y });
      if (Cx0 >= 0 && Cx1 >= 0 && Cx2 >= 0) {
      // ZoneNamedN(fill, "fill_pixel", true);
        F32 w1 = Cx1 / area;
        F32 w2 = Cx2 / area;
        F32 w3 = Cx0 / area;

        // @Note: We could do: interpolated_w = 1.f / interpolated_w to get proper depth
        // but why waste an instruction, the smaller the depth value the farther the object
        F32 interpolated_w = (1.f / p0.w) * w1 + (1.f / p1.w) * w2 + (1.f / p2.w) * w3;
        F32* depth = depth_buffer + (x + y * dst->x);
        if (*depth < interpolated_w) {
          *depth = interpolated_w;
          F32 invw0 = (w1 / p0.w);
          F32 invw1 = (w2 / p1.w);
          F32 invw2 = (w3 / p2.w);

          // Vec3 norm = (norm0 * invw0 + norm1 * invw1 + norm2 * invw2) / interpolated_w;
          F32 u = tex0.x * invw0 + tex1.x * invw1 + tex2.x * invw2;
          F32 v = tex0.y * invw0 + tex1.y * invw1 + tex2.y * invw2;
          {
            u /= interpolated_w;
            v /= interpolated_w;
            u = u - floor(u);
            v = v - floor(v);
            u = u * (src->x - 1);
            v = v * (src->y - 1);
          }
          S64 ui = (S64)(u);
          S64 vi = (S64)(v);
          //F32 udiff = u - (F32)ui;
          //F32 vdiff = v - (F32)vi;
          // Origin UV (0,0) is in bottom left
          U32 *dst_pixel = destination + x;
          U32 *pixel = src->pixels + (ui + (src->y - 1ll - vi) * src->x);

          Vec4 result_color; {
            U32 c = *pixel;
            F32 a = ((c & 0xff000000) >> 24) / 255.f;
            F32 b = ((c & 0x00ff0000) >> 16) / 255.f;
            F32 g = ((c & 0x0000ff00) >> 8)  / 255.f;
            F32 r = ((c & 0x000000ff) >> 0)  / 255.f;
            r*=r;
            g*=g;
            b*=b;
            result_color = { r,g,b,a };
          }

          Vec4 dst_color; {
            U32 c = *dst_pixel;
            F32 a = ((c & 0xff000000) >> 24) / 255.f;
            F32 b = ((c & 0x00ff0000) >> 16) / 255.f;
            F32 g = ((c & 0x0000ff00) >> 8)  / 255.f;
            F32 r = ((c & 0x000000ff) >> 0)  / 255.f;
            r*=r; g*=g; b*=b;
            dst_color = { r,g,b,a };
          }

#if 0
          Vec3 light_color = vec3(0.8,0.8,1);
          constexpr F32 ambient_strength = 0.1f; {
            Vec3 ambient = ambient_strength * light_color;
            Vec3 diffuse = clamp_bot(0.f, -dot(norm, light_direction)) * light_color;
            result_color.rgb *= (ambient+diffuse);
          }
#endif

          result_color = premultiplied_alpha(dst_color, result_color);
          result_color = almost_linear_to_srgb(result_color);
          U32 color32 = vec4_to_u32abgr(result_color);

          *dst_pixel = color32;
        }
      }
    }
    destination += dst->x;
  }
  filled_pixel_cycles += __rdtsc() - fill_pixels_begin;
  filled_pixel_count  += (max_x - min_x)*(max_y - min_y);
}

function
void draw_triangle_nearest_b(Bitmap* dst, F32 *depth_buffer, Bitmap *src, Vec3 light_direction,
                           Vec4 p0,   Vec4 p1,   Vec4 p2,
                           Vec2 tex0, Vec2 tex1, Vec2 tex2,
                           Vec3 norm0, Vec3 norm1, Vec3 norm2) {
  if(src->pixels == 0) return;
  U64 fill_pixels_begin = __rdtsc();

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

  U32 *destination = dst->pixels + dst->x*min_y;
  F32 area = (p1.y - p0.y) * (p2.x - p0.x) - (p1.x - p0.x) * (p2.y - p0.y);
  for (S64 y = min_y; y < max_y; y++) {
    F32 Cx0 = Cy0;
    F32 Cx1 = Cy1;
    F32 Cx2 = Cy2;
    for (S64 x = min_x; x < max_x; x++) {
      if (Cx0 >= 0 && Cx1 >= 0 && Cx2 >= 0) {
      // ZoneNamedN(fill, "fill_pixel", true);
        F32 w1 = Cx1 / area;
        F32 w2 = Cx2 / area;
        F32 w3 = Cx0 / area;

        // @Note: We could do: interpolated_w = 1.f / interpolated_w to get proper depth
        // but why waste an instruction, the smaller the depth value the farther the object
        F32 interpolated_w = (1.f / p0.w) * w1 + (1.f / p1.w) * w2 + (1.f / p2.w) * w3;
        F32* depth = depth_buffer + (x + y * dst->x);
        if (*depth < interpolated_w) {
          *depth = interpolated_w;
          F32 invw0 = (w1 / p0.w);
          F32 invw1 = (w2 / p1.w);
          F32 invw2 = (w3 / p2.w);

          // Vec3 norm = (norm0 * invw0 + norm1 * invw1 + norm2 * invw2) / interpolated_w;
          F32 u = tex0.x * invw0 + tex1.x * invw1 + tex2.x * invw2;
          F32 v = tex0.y * invw0 + tex1.y * invw1 + tex2.y * invw2;
          {
            u /= interpolated_w;
            v /= interpolated_w;
            u = u - floor(u);
            v = v - floor(v);
            u = u * (src->x - 1);
            v = v * (src->y - 1);
          }
          S64 ui = (S64)(u);
          S64 vi = (S64)(v);
          //F32 udiff = u - (F32)ui;
          //F32 vdiff = v - (F32)vi;
          // Origin UV (0,0) is in bottom left
          U32 *dst_pixel = destination + x;
          U32 *pixel = src->pixels + (ui + (src->y - 1ll - vi) * src->x);

          Vec4 result_color; {
            U32 c = *pixel;
            F32 a = ((c & 0xff000000) >> 24) / 255.f;
            F32 b = ((c & 0x00ff0000) >> 16) / 255.f;
            F32 g = ((c & 0x0000ff00) >> 8)  / 255.f;
            F32 r = ((c & 0x000000ff) >> 0)  / 255.f;
            r*=r;
            g*=g;
            b*=b;
            result_color = { r,g,b,a };
          }

          Vec4 dst_color; {
            U32 c = *dst_pixel;
            F32 a = ((c & 0xff000000) >> 24) / 255.f;
            F32 b = ((c & 0x00ff0000) >> 16) / 255.f;
            F32 g = ((c & 0x0000ff00) >> 8)  / 255.f;
            F32 r = ((c & 0x000000ff) >> 0)  / 255.f;
            r*=r; g*=g; b*=b;
            dst_color = { r,g,b,a };
          }

#if 0
          Vec3 light_color = vec3(0.8,0.8,1);
          constexpr F32 ambient_strength = 0.1f; {
            Vec3 ambient = ambient_strength * light_color;
            Vec3 diffuse = clamp_bot(0.f, -dot(norm, light_direction)) * light_color;
            result_color.rgb *= (ambient+diffuse);
          }
#endif

          result_color = premultiplied_alpha(dst_color, result_color);
          result_color = almost_linear_to_srgb(result_color);
          U32 color32 = vec4_to_u32abgr(result_color);

          *dst_pixel = color32;
        }
      }
      Cx0 += dy10;
      Cx1 += dy21;
      Cx2 += dy02;
    }
    Cy0 -= dx10;
    Cy1 -= dx21;
    Cy2 -= dx02;
    destination += dst->x;
  }
  filled_pixel_cycles += __rdtsc() - fill_pixels_begin;
  filled_pixel_count  += (max_x - min_x)*(max_y - min_y);
}


function
void draw_triangle_bilinear(Bitmap* dst, F32 *depth_buffer, Bitmap *src, Vec3 light_direction,
                            Vec4 p0,   Vec4 p1,   Vec4 p2,
                            Vec2 tex0, Vec2 tex1, Vec2 tex2,
                            Vec3 norm0, Vec3 norm1, Vec3 norm2) {
  if(src->pixels == 0) return;
  U64 fill_pixels_begin = __rdtsc();
  F32 min_x1 = (F32)(min(p0.x, min(p1.x, p2.x)));
  F32 min_y1 = (F32)(min(p0.y, min(p1.y, p2.y)));
  F32 max_x1 = (F32)(max(p0.x, max(p1.x, p2.x)));
  F32 max_y1 = (F32)(max(p0.y, max(p1.y, p2.y)));

  S64 min_x = (S64)clamp_bot(0.f, floor(min_x1));
  S64 min_y = (S64)clamp_bot(0.f, floor(min_y1));
  S64 max_x = (S64)clamp_top((F32)dst->x, ceil(max_x1));
  S64 max_y = (S64)clamp_top((F32)dst->y, ceil(max_y1));

  if (min_y >= max_y) return;
  if (min_x >= max_x) return;


  F32 area = edge_function(p0, p1, p2);
  for (S64 y = min_y; y < max_y; y++) {
    for (S64 x = min_x; x < max_x; x++) {
      F32 edge0 = edge_function(p0, p1, { (F32)x,(F32)y });
      F32 edge1 = edge_function(p1, p2, { (F32)x,(F32)y });
      F32 edge2 = edge_function(p2, p0, { (F32)x,(F32)y });

      if (edge0 >= 0 && edge1 >= 0 && edge2 >= 0) {
        F32 w1 = edge1 / area;
        F32 w2 = edge2 / area;
        F32 w3 = edge0 / area;
        F32 interpolated_w = (1.f / p0.w) * w1 + (1.f / p1.w) * w2 + (1.f / p2.w) * w3;

        // @Note: We could do: interpolated_w = 1.f / interpolated_w to get proper depth
        // but why waste an instruction, the smaller the depth value the farther the object
        F32* depth = depth_buffer + (x + y * dst->x);
        if (*depth < interpolated_w) {
          *depth = interpolated_w;
          F32 invw0 = (w1 / p0.w);
          F32 invw1 = (w2 / p1.w);
          F32 invw2 = (w3 / p2.w);

          Vec3 norm = (norm0 * invw0 + norm1 * invw1 + norm2 * invw2) / interpolated_w;
          F32 u = tex0.x * invw0 + tex1.x * invw1 + tex2.x * invw2;
          F32 v = tex0.y * invw0 + tex1.y * invw1 + tex2.y * invw2;
          {
            u /= interpolated_w;
            v /= interpolated_w;
            u = u - floor(u);
            v = v - floor(v);
            u = u * (src->x - 1);
            v = v * (src->y - 1);
          }

          S64 ui = (S64)(u);
          S64 vi = (S64)(v);
          F32 udiff = u - (F32)ui;
          F32 vdiff = v - (F32)vi;
          // Origin UV (0,0) is in bottom left
          U32 *pixel = src->pixels + (ui + (src->y - 1ll - vi) * src->x);
          U32 *dst_pixel = dst->pixels + (x + y * dst->x);

#if 0
          Vec4 result_color = vec4abgr(*pixel);
#else
          Vec4 pixelx1y1 = vec4abgr(*pixel);
          Vec4 pixelx2y1 = vec4abgr(*(pixel + 1));
          Vec4 pixelx1y2 = vec4abgr(*(pixel - src->x));
          Vec4 pixelx2y2 = vec4abgr(*(pixel + 1 - src->x));
          pixelx1y1 = srgb_to_almost_linear(pixelx1y1);
          pixelx2y1 = srgb_to_almost_linear(pixelx2y1);
          pixelx1y2 = srgb_to_almost_linear(pixelx1y2);
          pixelx2y2 = srgb_to_almost_linear(pixelx2y2);
          Vec4 blendx1 = lerp(pixelx1y1, pixelx2y1, udiff);
          Vec4 blendx2 = lerp(pixelx1y2, pixelx2y2, udiff);
          Vec4 result_color = lerp(blendx1, blendx2, vdiff);
#endif

          Vec3 light_color = vec3(0.8,0.8,1);
          constexpr F32 ambient_strength = 0.1f; {
            Vec3 ambient = ambient_strength * light_color;
            Vec3 diffuse = clamp_bot(0.f, -dot(norm, light_direction)) * light_color;
            result_color.rgb *= (ambient+diffuse);
          }

          Vec4 dst_color = vec4abgr(*dst_pixel);
          dst_color = srgb_to_almost_linear(dst_color);
          result_color = premultiplied_alpha(dst_color, result_color);
          result_color = almost_linear_to_srgb(result_color);
          U32 color32 = vec4_to_u32abgr(result_color);

          *dst_pixel = color32;
        }
      }
    }
  }
  filled_pixel_cycles += __rdtsc() - fill_pixels_begin;
  filled_pixel_count  += (max_x - min_x)*(max_y - min_y);
}

function
void draw_triangle_nearest_simd_with_overloads(Bitmap* dst, F32 *depth_buffer, Bitmap *src, Vec3 light_direction,
                           Vec4 p0,   Vec4 p1,   Vec4 p2,
                           Vec2 tex0, Vec2 tex1, Vec2 tex2,
                           Vec3 norm0, Vec3 norm1, Vec3 norm2) {
  if(src->pixels == 0) return;
  U64 fill_pixels_begin = __rdtsc();

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

  Vec8 var255 = vec8(255);
  Vec8 zero8 = vec8(0);
  Vec8 var1 = vec8(1);
  Vec8I var0i = vec8i(0);
  Vec8I var1i = vec8i(1);
  // Vec8I var07i = vec8i(0,1,2,3,4,5,6,7);
  Vec8 var07 = vec8(0,1,2,3,4,5,6,7);
  Vec8 var1_8 = vec8(1,2,3,4,5,6,7,8);
  Vec8 Dy10 = vec8(dy10) * var1_8;
  Vec8 Dy21 = vec8(dy21) * var1_8;
  Vec8 Dy02 = vec8(dy02) * var1_8;

  Vec8 iw_term0 = vec8(1.f / p0.w);
  Vec8 iw_term1 = vec8(1.f / p1.w);
  Vec8 iw_term2 = vec8(1.f / p2.w);

  U32 *destination = dst->pixels + dst->x*min_y;
  F32 area = (p1.y - p0.y) * (p2.x - p0.x) - (p1.x - p0.x) * (p2.y - p0.y);
  Vec8 area8 = vec8(area);

  for (S64 y = min_y; y < max_y; y++) {
    Vec8 Cx0 = vec8(Cy0);
    Vec8 Cx1 = vec8(Cy1);
    Vec8 Cx2 = vec8(Cy2);

    for (S64 x8 = min_x; x8 < max_x; x8+=8) {
      Cx0 = vec8(Cx0[7]) + Dy10;
      Cx1 = vec8(Cx1[7]) + Dy21;
      Cx2 = vec8(Cx2[7]) + Dy02;

      Vec8 should_fill;
      {
        Vec8 a = (vec8(x8) + var07);
        Vec8 b = vec8(max_x);
        should_fill = a < b;
        should_fill = should_fill & (Cx0 >= zero8 & Cx1 >= zero8 & Cx2 >= zero8);
      }

      Vec8 w0 = Cx1 / area8;
      Vec8 w1 = Cx2 / area8;
      Vec8 w2 = Cx0 / area8;

      // @Note: We could do: interpolated_w = 1.f / interpolated_w to get proper depth
      // but why waste an instruction, the smaller the depth value the farther the object
      Vec8 interpolated_w = iw_term0 * w0 + iw_term1 * w1 + iw_term2 * w2;
      F32 *depth_pointer = (depth_buffer + (x8 + y * dst->x));
      Vec8 depth = loadu8(depth_pointer);
      should_fill = should_fill & (depth < interpolated_w);


      Vec8 invw0 = (w0 / vec8(p0.w));
      Vec8 invw1 = (w1 / vec8(p1.w));
      Vec8 invw2 = (w2 / vec8(p2.w));

      Vec8 u = vec8(tex0.x) * invw0 + vec8(tex1.x) * invw1 + vec8(tex2.x) * invw2;
      Vec8 v = vec8(tex0.y) * invw0 + vec8(tex1.y) * invw1 + vec8(tex2.y) * invw2;
      u /= interpolated_w;
      v /= interpolated_w;
      u = u - floor8(u);
      v = v - floor8(v);
      u = u * vec8(src->x - 1);
      v = v * vec8(src->y - 1);
      Vec8I ui = convert_vec8_to_vec8i(u);
      Vec8I vi = convert_vec8_to_vec8i(v);

      // Origin UV (0,0) is in bottom left
      _mm256_maskstore_epi32((int *)depth_pointer, should_fill.simd, interpolated_w.simd);
      Vec8I indices = ui + ((vec8i(src->y) - var1i - vi) * vec8i(src->x));
      S32 size = src->x * src->y;
      indices.simd = _mm256_min_epi32(_mm256_set1_ps(size), indices.simd);
      indices.simd = _mm256_max_epi32(var0i.simd, indices.simd);

      //
      // Fetch and calculate texel values
      //
      Vec8I pixel;
      if(should_fill[0]) pixel.e[0] = src->pixels[indices.e[0]];
      if(should_fill[1]) pixel.e[1] = src->pixels[indices.e[1]];
      if(should_fill[2]) pixel.e[2] = src->pixels[indices.e[2]];
      if(should_fill[3]) pixel.e[3] = src->pixels[indices.e[3]];
      if(should_fill[4]) pixel.e[4] = src->pixels[indices.e[4]];
      if(should_fill[5]) pixel.e[5] = src->pixels[indices.e[5]];
      if(should_fill[6]) pixel.e[6] = src->pixels[indices.e[6]];
      if(should_fill[7]) pixel.e[7] = src->pixels[indices.e[7]];

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
      Vec8I dst_pixel = {_mm256_maskload_epi32((const int *)dst_memory, should_fill.simd)};

      Vec8I dst_i_a = dst_pixel & vec8i(0xff000000);
      Vec8I dst_i_b = dst_pixel & vec8i(0x00ff0000);
      Vec8I dst_i_g = dst_pixel & vec8i(0x0000ff00);
      Vec8I dst_i_r = dst_pixel & vec8i(0x000000ff);

      dst_i_a = dst_i_a >> 24;
      dst_i_a = dst_i_a &  vec8i(0x000000ff);
      dst_i_b = dst_i_b >> 16 ;
      dst_i_g = dst_i_g >> 8;

      Vec8 dst_a = convert_vec8i_to_vec8(dst_i_a) / var255;
      Vec8 dst_b = convert_vec8i_to_vec8(dst_i_b) / var255;
      Vec8 dst_g = convert_vec8i_to_vec8(dst_i_g) / var255;
      Vec8 dst_r = convert_vec8i_to_vec8(dst_i_r) / var255;

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

      // Convert to integer format
      dst_r = dst_r * var255;
      dst_g = dst_g * var255;
      dst_b = dst_b * var255;
      dst_a = dst_a * var255;

      Vec8I dst_r_int = convert_vec8_to_vec8i(dst_r);
      Vec8I dst_g_int = convert_vec8_to_vec8i(dst_g);
      Vec8I dst_b_int = convert_vec8_to_vec8i(dst_b);
      Vec8I dst_a_int = convert_vec8_to_vec8i(dst_a);

      Vec8I dst_int_a_shifted = {_mm256_slli_epi32(dst_a_int.simd, 24)};
      Vec8I dst_int_b_shifted = {_mm256_slli_epi32(dst_b_int.simd, 16)};
      Vec8I dst_int_g_shifted = {_mm256_slli_epi32(dst_g_int.simd, 8)};
      Vec8I dst_int_r_shifted = dst_r_int;

      Vec8I packed_abgr0 = {_mm256_or_si256(dst_int_a_shifted.simd, dst_int_b_shifted.simd)};
      Vec8I packed_abgr1 = {_mm256_or_si256(packed_abgr0.simd, dst_int_g_shifted.simd)};
      Vec8I packed_abgr2 = {_mm256_or_si256(packed_abgr1.simd, dst_int_r_shifted.simd)};

      _mm256_maskstore_epi32((int *)dst_memory, should_fill.simd, packed_abgr2.simd);

    }
    Cy0 -= dx10;
    Cy1 -= dx21;
    Cy2 -= dx02;
    destination += dst->x;
  }
  filled_pixel_cycles += __rdtsc() - fill_pixels_begin;
  filled_pixel_count  += (max_x - min_x)*(max_y - min_y);
}

function
void draw_triangle_nearest_simd_without_overloads(Bitmap* dst, F32 *depth_buffer, Bitmap *src, Vec3 light_direction,
                           Vec4 p0,   Vec4 p1,   Vec4 p2,
                           Vec2 tex0, Vec2 tex1, Vec2 tex2,
                           Vec3 norm0, Vec3 norm1, Vec3 norm2) {
  if(src->pixels == 0) return;
  U64 fill_pixels_begin = __rdtsc();

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
  F32x8 var1 = _mm256_set1_ps(1);
  F32x8 var_max_x = _mm256_set1_ps(max_x);
  F32x8 var07 = _mm256_set_ps(7,6,5,4,3,2,1,0);

  F32x8 var_1_8 = _mm256_set_ps(8,7,6,5,4,3,2,1);
  F32x8 Dy10 = _mm256_mul_ps(_mm256_set1_ps(dy10), var_1_8);
  F32x8 Dy21 = _mm256_mul_ps(_mm256_set1_ps(dy21), var_1_8);
  F32x8 Dy02 = _mm256_mul_ps(_mm256_set1_ps(dy02), var_1_8);

  F32x8 var_src_x_minus_one = _mm256_set1_ps(src->x-1);
  F32x8 var_src_y_minus_one = _mm256_set1_ps(src->y-1);
  S32x8 var_src_y_minus_one_int = _mm256_set1_epi32(src->y-1);
  S32x8 var_src_x_int = _mm256_set1_epi32(src->x);

  S32x8 var_0xff000000 = _mm256_set1_epi32(0xff000000);
  S32x8 var_0x00ff0000 = _mm256_set1_epi32(0x00ff0000);
  S32x8 var_0x0000ff00 = _mm256_set1_epi32(0x0000ff00);
  S32x8 var_0x000000ff = _mm256_set1_epi32(0x000000ff);

  F32x8 var_255 = _mm256_set1_ps(255);
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

  for (S64 y = min_y; y < max_y; y++) {
    F32x8 Cx0 = _mm256_set1_ps(Cy0);
    F32x8 Cx1 = _mm256_set1_ps(Cy1);
    F32x8 Cx2 = _mm256_set1_ps(Cy2);

    for (S64 x8 = min_x; x8 < max_x; x8+=8) {
      {
        F32x8 i0 = _mm256_set1_ps(I(Cx0, 7));
        Cx0 = _mm256_add_ps(i0, Dy10);

        F32x8 i2 = _mm256_set1_ps(I(Cx1, 7));
        Cx1 = _mm256_add_ps(i2, Dy21);

        F32x8 i4 = _mm256_set1_ps(I(Cx2, 7));
        Cx2 = _mm256_add_ps(i4, Dy02);
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

      S32x8 indices1 = _mm256_sub_epi32(var_src_y_minus_one_int, vi);
      S32x8 indices3 = _mm256_mullo_epi32(var_src_x_int, indices1);
      S32x8 indices  = _mm256_add_epi32(indices3, ui);

      //
      // Fetch and calculate texel values
      //
      S32x8 pixel;
      if(I(should_fill, 0)) Is(pixel, 0) = src->pixels[Is(indices, 0)];
      if(I(should_fill, 1)) Is(pixel, 1) = src->pixels[Is(indices, 1)];
      if(I(should_fill, 2)) Is(pixel, 2) = src->pixels[Is(indices, 2)];
      if(I(should_fill, 3)) Is(pixel, 3) = src->pixels[Is(indices, 3)];
      if(I(should_fill, 4)) Is(pixel, 4) = src->pixels[Is(indices, 4)];
      if(I(should_fill, 5)) Is(pixel, 5) = src->pixels[Is(indices, 5)];
      if(I(should_fill, 6)) Is(pixel, 6) = src->pixels[Is(indices, 6)];
      if(I(should_fill, 7)) Is(pixel, 7) = src->pixels[Is(indices, 7)];

      S32x8 texel_i_a = _mm256_and_si256(pixel, var_0xff000000);
      S32x8 texel_i_b = _mm256_and_si256(pixel, var_0x00ff0000);
      S32x8 texel_i_g = _mm256_and_si256(pixel, var_0x0000ff00);
      S32x8 texel_i_r = _mm256_and_si256(pixel, var_0x000000ff);

      // Alpha is done this way because signed integer shift is weird
      // When sign bit is set it sets all bits that we shift the sign through
      // So first we shift
      texel_i_a = _mm256_srai_epi32(texel_i_a, 24);
      texel_i_a = _mm256_and_si256(texel_i_a, var_0x000000ff);
      texel_i_b = _mm256_srai_epi32(texel_i_b, 16);
      texel_i_g = _mm256_srai_epi32(texel_i_g, 8 );
      texel_i_r = _mm256_srai_epi32(texel_i_r, 0 );

      F32x8 texel_a0 = _mm256_cvtepi32_ps(texel_i_a);
      F32x8 texel_b0 = _mm256_cvtepi32_ps(texel_i_b);
      F32x8 texel_g0 = _mm256_cvtepi32_ps(texel_i_g);
      F32x8 texel_r0 = _mm256_cvtepi32_ps(texel_i_r);

      F32x8 texel_a1 = _mm256_div_ps(texel_a0, var_255);
      F32x8 texel_b1 = _mm256_div_ps(texel_b0, var_255);
      F32x8 texel_g1 = _mm256_div_ps(texel_g0, var_255);
      F32x8 texel_r1 = _mm256_div_ps(texel_r0, var_255);

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

      dst_a = _mm256_div_ps(dst_a, var255);
      dst_b = _mm256_div_ps(dst_b, var255);
      dst_g = _mm256_div_ps(dst_g, var255);
      dst_r = _mm256_div_ps(dst_r, var255);

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
      S32x8 packed_abgr1 = _mm256_or_si256(packed_abgr0, dst_int_g_shifted);
      S32x8 packed_abgr2 = _mm256_or_si256(packed_abgr1, dst_int_r_shifted);

      _mm256_maskstore_epi32((int *)dst_memory, should_fill, packed_abgr2);

    }
    Cy0 -= dx10;
    Cy1 -= dx21;
    Cy2 -= dx02;
    destination += dst->x;
  }
  filled_pixel_cycles += __rdtsc() - fill_pixels_begin;
  filled_pixel_count      += (max_x - min_x)*(max_y - min_y);
}


function
void draw_triangle_nearest_final(Bitmap* dst, F32 *depth_buffer, Bitmap *src, Vec3 light_direction,
                           Vec4 p0,   Vec4 p1,   Vec4 p2,
                           Vec2 tex0, Vec2 tex1, Vec2 tex2,
                           Vec3 norm0, Vec3 norm1, Vec3 norm2) {
  if(src->pixels == 0) return;

  U64 fill_pixels_begin = __rdtsc();

  F32 region_min_x = 0;
  F32 region_min_y = 0;
  F32 region_max_x = dst->x;
  F32 region_max_y = dst->y;

  F32 min_x1 = (F32)(min(p0.x, min(p1.x, p2.x)));
  F32 min_y1 = (F32)(min(p0.y, min(p1.y, p2.y)));
  F32 max_x1 = (F32)(max(p0.x, max(p1.x, p2.x)));
  F32 max_y1 = (F32)(max(p0.y, max(p1.y, p2.y)));

  S64 min_x = (S64)max(region_min_x, floor(min_x1));
  S64 min_y = (S64)max(region_min_y, floor(min_y1));
  S64 max_x = (S64)min(region_max_x, ceil(max_x1));
  S64 max_y = (S64)min(region_max_y, ceil(max_y1));

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

  for (S64 y = min_y; y < max_y; y++) {
    F32x8 Y = _mm256_set1_ps(y);
    for (S64 x8 = min_x; x8 < max_x; x8+=8) {
      F32x8 X = _mm256_add_ps(_mm256_set1_ps(x8), var07);

      // Compute the edges
      // F32x8 edge0 = (p1.y - p0.y) * (p.x - p0.x) - (p1.x - p0.x) * (p.y - p0.y);
      F32x8 px_minus_0x = _mm256_sub_ps(X, p0_x);
      F32x8 py_minus_0y = _mm256_sub_ps(Y, p0_y);
      F32x8 right0 = _mm256_mul_ps(_dx10, py_minus_0y);
      F32x8 edge0 = _mm256_fmsub_ps(_dy10, px_minus_0x, right0);

      // F32 result = (p2.y - p1.y) * (p.x - p1.x) - (p2.x - p1.x) * (p.y - p1.y);
      F32x8 px_minus_1x = _mm256_sub_ps(X, p1_x);
      F32x8 py_minus_1y = _mm256_sub_ps(Y, p1_y);
      F32x8 right1 = _mm256_mul_ps(_dx21, py_minus_1y);
      F32x8 edge1 = _mm256_fmsub_ps(_dy21, px_minus_1x, right1);

      // F32 result = (p0.y - p2.y) * (p.x - p2.x) - (p0.x - p2.x) * (p.y - p2.y);
      F32x8 px_minus_2x = _mm256_sub_ps(X, p2_x);
      F32x8 py_minus_2y = _mm256_sub_ps(Y, p2_y);
      F32x8 right2 = _mm256_mul_ps(_dx02, py_minus_2y);
      F32x8 edge2 = _mm256_fmsub_ps(_dy02, px_minus_2x, right2);

      F32x8 should_fill;
      F32x8 test_if_x_should_be_clipped = _mm256_cmp_ps(X, var_max_x, _CMP_LT_OQ);
      F32x8 test_if_pixel_inside_edge_using_dot_result0  = _mm256_cmp_ps(edge0, var0, _CMP_GE_OQ);
      F32x8 test_if_pixel_inside_edge_using_dot_result1  = _mm256_cmp_ps(edge1, var0, _CMP_GE_OQ);
      F32x8 test_if_pixel_inside_edge_using_dot_result2  = _mm256_cmp_ps(edge2, var0, _CMP_GE_OQ);
      F32x8 dot_result_combination0  = _mm256_and_ps(test_if_pixel_inside_edge_using_dot_result0, test_if_pixel_inside_edge_using_dot_result1);
      F32x8 dot_result_combination1 = _mm256_and_ps(dot_result_combination0, test_if_pixel_inside_edge_using_dot_result2);
      should_fill = _mm256_and_ps(test_if_x_should_be_clipped, dot_result_combination1);

      F32x8 w0 = _mm256_mul_ps(edge1, inv_area8);
      F32x8 w1 = _mm256_mul_ps(edge2, inv_area8);
      F32x8 w2 = _mm256_mul_ps(edge0, inv_area8);

      // @Todo: Turn this into 1 / interpolated_w, turns out in theory it should be
      // more performant but couldn't make it work

      // @Old_Note: We could do: interpolated_w = 1.f / interpolated_w to get proper depth
      // but why waste an instruction, the smaller the depth value the farther the object
      F32x8 interpolated_w = _mm256_mul_ps(one_over_p0w, w0);
      interpolated_w = _mm256_fmadd_ps(one_over_p1w, w1, interpolated_w);
      interpolated_w = _mm256_fmadd_ps(one_over_p2w, w2, interpolated_w);

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

      F32x8 u0 = _mm256_mul_ps(var_tex0x, invw0);
      u0 = _mm256_fmadd_ps(var_tex1x, invw1, u0);
      u0 = _mm256_fmadd_ps(var_tex2x, invw2, u0);

      F32x8 v0 = _mm256_mul_ps(var_tex0y, invw0);
      v0 = _mm256_fmadd_ps(var_tex1y, invw1, v0);
      v0 = _mm256_fmadd_ps(var_tex2y, invw2, v0);

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

      //
      // Fetch and calculate texel values
      //
      S32x8 indices_to_fetch0 = _mm256_sub_epi32(var_src_y_minus_one_int, vi);
      S32x8 indices_to_fetch1 = _mm256_mullo_epi32(var_src_x_int, indices_to_fetch0);
      S32x8 indices_to_fetch2  = _mm256_add_epi32(indices_to_fetch1, ui);
      S32x8 indices_to_fetch3 = _mm256_and_si256(indices_to_fetch2, should_fill);

      S32x8 pixel = _mm256_set_epi32(
        src->pixels[_mm256_extract_epi32(indices_to_fetch3, 7)],
        src->pixels[_mm256_extract_epi32(indices_to_fetch3, 6)],
        src->pixels[_mm256_extract_epi32(indices_to_fetch3, 5)],
        src->pixels[_mm256_extract_epi32(indices_to_fetch3, 4)],
        src->pixels[_mm256_extract_epi32(indices_to_fetch3, 3)],
        src->pixels[_mm256_extract_epi32(indices_to_fetch3, 2)],
        src->pixels[_mm256_extract_epi32(indices_to_fetch3, 1)],
        src->pixels[_mm256_extract_epi32(indices_to_fetch3, 0)]
      );

      S32x8 texel_i_a = _mm256_and_si256(pixel, var_0xff000000);
      S32x8 texel_i_b = _mm256_and_si256(pixel, var_0x00ff0000);
      S32x8 texel_i_g = _mm256_and_si256(pixel, var_0x0000ff00);
      S32x8 texel_i_r = _mm256_and_si256(pixel, var_0x000000ff);

      texel_i_a = _mm256_srli_epi32(texel_i_a, 24);
      texel_i_b = _mm256_srli_epi32(texel_i_b, 16);
      texel_i_g = _mm256_srli_epi32(texel_i_g, 8 );

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

      S32x8 dst_i_a1 = _mm256_srli_epi32(dst_i_a0, 24);
      S32x8 dst_i_b1 = _mm256_srli_epi32(dst_i_b0, 16);
      S32x8 dst_i_g1 = _mm256_srli_epi32(dst_i_g0, 8);
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
        F32x8 inv_texel_a = _mm256_sub_ps(var1,texel_a1);
        dst_r = _mm256_fmadd_ps(inv_texel_a, dst_r, texel_r1);
        dst_g = _mm256_fmadd_ps(inv_texel_a, dst_g, texel_g1);
        dst_b = _mm256_fmadd_ps(inv_texel_a, dst_b, texel_b1);
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

  filled_pixel_cycles += __rdtsc() - fill_pixels_begin;
  filled_pixel_count  += (max_x - min_x)*(max_y - min_y);
}
