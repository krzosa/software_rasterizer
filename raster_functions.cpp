
FUNCTION void draw_triangle_PERSPECTIVE_CORRECT_INTERPOLATION_off_BILINEAR_BLEND_off (Image* dst, float *depth_buffer, Image *src, 
                  Vec4 p0,   Vec4 p1,   Vec4 p2,
                  Vec2 tex0, Vec2 tex1, Vec2 tex2) {
  float min_x1 = (float)(MIN(p0.x, MIN(p1.x, p2.x)));
  float min_y1 = (float)(MIN(p0.y, MIN(p1.y, p2.y)));
  float max_x1 = (float)(MAX(p0.x, MAX(p1.x, p2.x)));
  float max_y1 = (float)(MAX(p0.y, MAX(p1.y, p2.y)));
  I64 min_x = (I64)MAX(0, floor(min_x1));
  I64 min_y = (I64)MAX(0, floor(min_y1));
  I64 max_x = (I64)MIN(dst->x, ceil(max_x1));
  I64 max_y = (I64)MIN(dst->y, ceil(max_y1));

  float area = edge_function(p0, p1, p2);
  for (I64 y = min_y; y < max_y; y++) {
    for (I64 x = min_x; x < max_x; x++) {
      float edge1 = edge_function(p0, p1, { (float)x,(float)y });
      float edge2 = edge_function(p1, p2, { (float)x,(float)y });
      float edge3 = edge_function(p2, p0, { (float)x,(float)y });

      if (edge1 >= 0 && edge2 >= 0 && edge3 >= 0) {
        float w1 = edge2 / area;
        float w2 = edge3 / area;
        float w3 = edge1 / area;
        float interpolated_z = (1.f / p0.w) * w1 + (1.f / p1.w) * w2 + (1.f / p2.w) * w3;
// PERSPECTIVE_CORRECT_INTERPOLATION 
        float u = tex0.x * w1 + tex1.x * w2 + tex2.x * w3;
        float v = tex0.y * w1 + tex1.y * w2 + tex2.y * w3;
 
        // @Note: We could do: interpolated_z = 1.f / interpolated_z to get proper depth
        // but why waste an instruction, the smaller the depth value the farther the object
        float* depth = depth_buffer + (x + y * dst->x);
        if (*depth < interpolated_z) {
          *depth = interpolated_z;
          u = u * (src->x - 2);
          v = v * (src->y - 2);
          I64 ui = (I64)(u);
          I64 vi = (I64)(v);
          float udiff = u - (float)ui;
          float vdiff = v - (float)vi;
          // Origin UV (0,0) is in bottom left
          U32* pixel = src->pixels + (ui + (src->y - 1ll - vi) * src->x);

// BILINEAR_BLEND 
          U32 color32 = *pixel;
 

          dst->pixels[x + y * dst->x] = color32;
        }
      }
    }
  }
  if (draw_rects) {
    draw_rect(dst, p0.x-4, p0.y-4, 8,8, 0x00ff0000);
    draw_rect(dst, p1.x-4, p1.y-4, 8,8, 0x0000ff00);
    draw_rect(dst, p2.x-4, p2.y-4, 8,8, 0x000000ff);
  }
}
FUNCTION void draw_triangle_PERSPECTIVE_CORRECT_INTERPOLATION_on_BILINEAR_BLEND_on (Image* dst, float *depth_buffer, Image *src, 
                  Vec4 p0,   Vec4 p1,   Vec4 p2,
                  Vec2 tex0, Vec2 tex1, Vec2 tex2) {
  float min_x1 = (float)(MIN(p0.x, MIN(p1.x, p2.x)));
  float min_y1 = (float)(MIN(p0.y, MIN(p1.y, p2.y)));
  float max_x1 = (float)(MAX(p0.x, MAX(p1.x, p2.x)));
  float max_y1 = (float)(MAX(p0.y, MAX(p1.y, p2.y)));
  I64 min_x = (I64)MAX(0, floor(min_x1));
  I64 min_y = (I64)MAX(0, floor(min_y1));
  I64 max_x = (I64)MIN(dst->x, ceil(max_x1));
  I64 max_y = (I64)MIN(dst->y, ceil(max_y1));

  float area = edge_function(p0, p1, p2);
  for (I64 y = min_y; y < max_y; y++) {
    for (I64 x = min_x; x < max_x; x++) {
      float edge1 = edge_function(p0, p1, { (float)x,(float)y });
      float edge2 = edge_function(p1, p2, { (float)x,(float)y });
      float edge3 = edge_function(p2, p0, { (float)x,(float)y });

      if (edge1 >= 0 && edge2 >= 0 && edge3 >= 0) {
        float w1 = edge2 / area;
        float w2 = edge3 / area;
        float w3 = edge1 / area;
        float interpolated_z = (1.f / p0.w) * w1 + (1.f / p1.w) * w2 + (1.f / p2.w) * w3;
// PERSPECTIVE_CORRECT_INTERPOLATION 
        float u = tex0.x * (w1 / p0.w) + tex1.x * (w2 / p1.w) + tex2.x * (w3 / p2.w);
        float v = tex0.y * (w1 / p0.w) + tex1.y * (w2 / p1.w) + tex2.y * (w3 / p2.w);
        u /= interpolated_z;
        v /= interpolated_z;
 
        // @Note: We could do: interpolated_z = 1.f / interpolated_z to get proper depth
        // but why waste an instruction, the smaller the depth value the farther the object
        float* depth = depth_buffer + (x + y * dst->x);
        if (*depth < interpolated_z) {
          *depth = interpolated_z;
          u = u * (src->x - 2);
          v = v * (src->y - 2);
          I64 ui = (I64)(u);
          I64 vi = (I64)(v);
          float udiff = u - (float)ui;
          float vdiff = v - (float)vi;
          // Origin UV (0,0) is in bottom left
          U32* pixel = src->pixels + (ui + (src->y - 1ll - vi) * src->x);

// BILINEAR_BLEND 
          Vec4 pixelx1y1 = srgb_to_almost_linear(v4abgr(*pixel));
          Vec4 pixelx2y1 = srgb_to_almost_linear(v4abgr(*(pixel + 1)));
          Vec4 pixelx1y2 = srgb_to_almost_linear(v4abgr(*(pixel - src->x)));
          Vec4 pixelx2y2 = srgb_to_almost_linear(v4abgr(*(pixel + 1 - src->x)));


          Vec4 blendx1 = lerp(pixelx1y1, pixelx2y1, udiff);
          Vec4 blendx2 = lerp(pixelx1y2, pixelx2y2, udiff);
          Vec4 result_color = lerp(blendx1, blendx2, vdiff);
          result_color = almost_linear_to_srgb(result_color);
          ASSERT(result_color.r <= 1 && result_color.g <= 1 && result_color.b <= 1);
          U32 color32 = color_to_u32abgr(result_color);
 

          dst->pixels[x + y * dst->x] = color32;
        }
      }
    }
  }
  if (draw_rects) {
    draw_rect(dst, p0.x-4, p0.y-4, 8,8, 0x00ff0000);
    draw_rect(dst, p1.x-4, p1.y-4, 8,8, 0x0000ff00);
    draw_rect(dst, p2.x-4, p2.y-4, 8,8, 0x000000ff);
  }
}
FUNCTION void draw_triangle_PERSPECTIVE_CORRECT_INTERPOLATION_off_BILINEAR_BLEND_on (Image* dst, float *depth_buffer, Image *src, 
                  Vec4 p0,   Vec4 p1,   Vec4 p2,
                  Vec2 tex0, Vec2 tex1, Vec2 tex2) {
  float min_x1 = (float)(MIN(p0.x, MIN(p1.x, p2.x)));
  float min_y1 = (float)(MIN(p0.y, MIN(p1.y, p2.y)));
  float max_x1 = (float)(MAX(p0.x, MAX(p1.x, p2.x)));
  float max_y1 = (float)(MAX(p0.y, MAX(p1.y, p2.y)));
  I64 min_x = (I64)MAX(0, floor(min_x1));
  I64 min_y = (I64)MAX(0, floor(min_y1));
  I64 max_x = (I64)MIN(dst->x, ceil(max_x1));
  I64 max_y = (I64)MIN(dst->y, ceil(max_y1));

  float area = edge_function(p0, p1, p2);
  for (I64 y = min_y; y < max_y; y++) {
    for (I64 x = min_x; x < max_x; x++) {
      float edge1 = edge_function(p0, p1, { (float)x,(float)y });
      float edge2 = edge_function(p1, p2, { (float)x,(float)y });
      float edge3 = edge_function(p2, p0, { (float)x,(float)y });

      if (edge1 >= 0 && edge2 >= 0 && edge3 >= 0) {
        float w1 = edge2 / area;
        float w2 = edge3 / area;
        float w3 = edge1 / area;
        float interpolated_z = (1.f / p0.w) * w1 + (1.f / p1.w) * w2 + (1.f / p2.w) * w3;
// PERSPECTIVE_CORRECT_INTERPOLATION 
        float u = tex0.x * w1 + tex1.x * w2 + tex2.x * w3;
        float v = tex0.y * w1 + tex1.y * w2 + tex2.y * w3;
 
        // @Note: We could do: interpolated_z = 1.f / interpolated_z to get proper depth
        // but why waste an instruction, the smaller the depth value the farther the object
        float* depth = depth_buffer + (x + y * dst->x);
        if (*depth < interpolated_z) {
          *depth = interpolated_z;
          u = u * (src->x - 2);
          v = v * (src->y - 2);
          I64 ui = (I64)(u);
          I64 vi = (I64)(v);
          float udiff = u - (float)ui;
          float vdiff = v - (float)vi;
          // Origin UV (0,0) is in bottom left
          U32* pixel = src->pixels + (ui + (src->y - 1ll - vi) * src->x);

// BILINEAR_BLEND 
          Vec4 pixelx1y1 = srgb_to_almost_linear(v4abgr(*pixel));
          Vec4 pixelx2y1 = srgb_to_almost_linear(v4abgr(*(pixel + 1)));
          Vec4 pixelx1y2 = srgb_to_almost_linear(v4abgr(*(pixel - src->x)));
          Vec4 pixelx2y2 = srgb_to_almost_linear(v4abgr(*(pixel + 1 - src->x)));


          Vec4 blendx1 = lerp(pixelx1y1, pixelx2y1, udiff);
          Vec4 blendx2 = lerp(pixelx1y2, pixelx2y2, udiff);
          Vec4 result_color = lerp(blendx1, blendx2, vdiff);
          result_color = almost_linear_to_srgb(result_color);
          ASSERT(result_color.r <= 1 && result_color.g <= 1 && result_color.b <= 1);
          U32 color32 = color_to_u32abgr(result_color);
 

          dst->pixels[x + y * dst->x] = color32;
        }
      }
    }
  }
  if (draw_rects) {
    draw_rect(dst, p0.x-4, p0.y-4, 8,8, 0x00ff0000);
    draw_rect(dst, p1.x-4, p1.y-4, 8,8, 0x0000ff00);
    draw_rect(dst, p2.x-4, p2.y-4, 8,8, 0x000000ff);
  }
}
FUNCTION void draw_triangle_PERSPECTIVE_CORRECT_INTERPOLATION_on_BILINEAR_BLEND_off (Image* dst, float *depth_buffer, Image *src, 
                  Vec4 p0,   Vec4 p1,   Vec4 p2,
                  Vec2 tex0, Vec2 tex1, Vec2 tex2) {
  float min_x1 = (float)(MIN(p0.x, MIN(p1.x, p2.x)));
  float min_y1 = (float)(MIN(p0.y, MIN(p1.y, p2.y)));
  float max_x1 = (float)(MAX(p0.x, MAX(p1.x, p2.x)));
  float max_y1 = (float)(MAX(p0.y, MAX(p1.y, p2.y)));
  I64 min_x = (I64)MAX(0, floor(min_x1));
  I64 min_y = (I64)MAX(0, floor(min_y1));
  I64 max_x = (I64)MIN(dst->x, ceil(max_x1));
  I64 max_y = (I64)MIN(dst->y, ceil(max_y1));

  float area = edge_function(p0, p1, p2);
  for (I64 y = min_y; y < max_y; y++) {
    for (I64 x = min_x; x < max_x; x++) {
      float edge1 = edge_function(p0, p1, { (float)x,(float)y });
      float edge2 = edge_function(p1, p2, { (float)x,(float)y });
      float edge3 = edge_function(p2, p0, { (float)x,(float)y });

      if (edge1 >= 0 && edge2 >= 0 && edge3 >= 0) {
        float w1 = edge2 / area;
        float w2 = edge3 / area;
        float w3 = edge1 / area;
        float interpolated_z = (1.f / p0.w) * w1 + (1.f / p1.w) * w2 + (1.f / p2.w) * w3;
// PERSPECTIVE_CORRECT_INTERPOLATION 
        float u = tex0.x * (w1 / p0.w) + tex1.x * (w2 / p1.w) + tex2.x * (w3 / p2.w);
        float v = tex0.y * (w1 / p0.w) + tex1.y * (w2 / p1.w) + tex2.y * (w3 / p2.w);
        u /= interpolated_z;
        v /= interpolated_z;
 
        // @Note: We could do: interpolated_z = 1.f / interpolated_z to get proper depth
        // but why waste an instruction, the smaller the depth value the farther the object
        float* depth = depth_buffer + (x + y * dst->x);
        if (*depth < interpolated_z) {
          *depth = interpolated_z;
          u = u * (src->x - 2);
          v = v * (src->y - 2);
          I64 ui = (I64)(u);
          I64 vi = (I64)(v);
          float udiff = u - (float)ui;
          float vdiff = v - (float)vi;
          // Origin UV (0,0) is in bottom left
          U32* pixel = src->pixels + (ui + (src->y - 1ll - vi) * src->x);

// BILINEAR_BLEND 
          U32 color32 = *pixel;
 

          dst->pixels[x + y * dst->x] = color32;
        }
      }
    }
  }
  if (draw_rects) {
    draw_rect(dst, p0.x-4, p0.y-4, 8,8, 0x00ff0000);
    draw_rect(dst, p1.x-4, p1.y-4, 8,8, 0x0000ff00);
    draw_rect(dst, p2.x-4, p2.y-4, 8,8, 0x000000ff);
  }
}