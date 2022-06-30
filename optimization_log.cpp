function
void draw_triangle_nearest_a(Bitmap* dst, F32 *depth_buffer, Bitmap *src, Vec3 light_direction,
                           Vec4 p0,   Vec4 p1,   Vec4 p2,
                           Vec2 tex0, Vec2 tex1, Vec2 tex2,
                           Vec3 norm0, Vec3 norm1, Vec3 norm2) {
  F32 min_x1 = (F32)(min(p0.x, min(p1.x, p2.x)));
  F32 min_y1 = (F32)(min(p0.y, min(p1.y, p2.y)));
  F32 max_x1 = (F32)(max(p0.x, max(p1.x, p2.x)));
  F32 max_y1 = (F32)(max(p0.y, max(p1.y, p2.y)));
  S64 min_x = (S64)max(0.f, floor(min_x1));
  S64 min_y = (S64)max(0.f, floor(min_y1));
  S64 max_x = (S64)min((F32)dst->x, ceil(max_x1));
  S64 max_y = (S64)min((F32)dst->y, ceil(max_y1));

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
}


function
void draw_triangle_nearest_b(Bitmap* dst, F32 *depth_buffer, Bitmap *src, Vec3 light_direction,
                           Vec4 p0,   Vec4 p1,   Vec4 p2,
                           Vec2 tex0, Vec2 tex1, Vec2 tex2,
                           Vec3 norm0, Vec3 norm1, Vec3 norm2) {
  // if(os.frame > 10) PROFILE_BEGIN(draw_triangle);
  // ZoneScopedN("draw_triangle");
  F32 min_x1 = (F32)(min(p0.x, min(p1.x, p2.x)));
  F32 min_y1 = (F32)(min(p0.y, min(p1.y, p2.y)));
  F32 max_x1 = (F32)(max(p0.x, max(p1.x, p2.x)));
  F32 max_y1 = (F32)(max(p0.y, max(p1.y, p2.y)));
  S64 min_x = (S64)max(0.f, floor(min_x1));
  S64 min_y = (S64)max(0.f, floor(min_y1));
  S64 max_x = (S64)min((F32)dst->x, ceil(max_x1));
  S64 max_y = (S64)min((F32)dst->y, ceil(max_y1));

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

     // if(os.frame > 10) PROFILE_END(draw_triangle);
}


function
void draw_triangle_bilinear(Bitmap* dst, F32 *depth_buffer, Bitmap *src, Vec3 light_direction,
                            Vec4 p0,   Vec4 p1,   Vec4 p2,
                            Vec2 tex0, Vec2 tex1, Vec2 tex2,
                            Vec3 norm0, Vec3 norm1, Vec3 norm2) {
  F32 min_x1 = (F32)(min(p0.x, min(p1.x, p2.x)));
  F32 min_y1 = (F32)(min(p0.y, min(p1.y, p2.y)));
  F32 max_x1 = (F32)(max(p0.x, max(p1.x, p2.x)));
  F32 max_y1 = (F32)(max(p0.y, max(p1.y, p2.y)));

  S64 min_x = (S64)clamp_bot(0.f, floor(min_x1));
  S64 min_y = (S64)clamp_bot(0.f, floor(min_y1));
  S64 max_x = (S64)clamp_top((F32)dst->x, ceil(max_x1));
  S64 max_y = (S64)clamp_top((F32)dst->y, ceil(max_y1));

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
}
