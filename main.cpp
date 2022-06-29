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

#define PREMULTIPLIED_ALPHA_BLENDING 1
#include "multimedia.cpp"
#include "profile.cpp"
#include "obj.cpp"

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

function
void draw_triangle_nearest(Bitmap* dst, F32 *depth_buffer, Bitmap *src, Vec3 light_direction,
                           Vec4 p0,   Vec4 p1,   Vec4 p2,
                           Vec2 tex0, Vec2 tex1, Vec2 tex2,
                           Vec3 norm0, Vec3 norm1, Vec3 norm2) {
  if(os.frame > 60) PROFILE_BEGIN(draw_triangle);
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

#if PREMULTIPLIED_ALPHA_BLENDING
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

          Vec3 light_color = vec3(0.8,0.8,1);
          constexpr F32 ambient_strength = 0.1f; {
            Vec3 ambient = ambient_strength * light_color;
            Vec3 diffuse = clamp_bot(0.f, -dot(norm, light_direction)) * light_color;
            result_color.rgb *= (ambient+diffuse);
          }


          result_color = premultiplied_alpha(dst_color, result_color);
          result_color = almost_linear_to_srgb(result_color);
          U32 color32 = vec4_to_u32abgr(result_color);
#else
          U32 color32 = *pixel;
#endif

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

  if(os.frame > 60) PROFILE_END(draw_triangle);
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

function
void draw_mesh(Render *r, String scene_name, Obj_Material *materials, Obj_Mesh *mesh, Vec3 *vertices, Vec2 *tex_coords, Vec3 *normals) {
  for (int i = 0; i < mesh->indices.len; i++) {
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
          in[in_count++].norm = lerp(prev->norm, curr->norm, t);
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
      r.camera_pos = vec3(0,0,-2);
      obj = sponza;
    } break;
    case Scene_Count:
    invalid_default_case;
  }
  scene = (Scene)(((int)scene + 1) % Scene_Count);
}

function void
windows_log(Log_Kind kind, String string, char *file, int line){
  OutputDebugStringA((char *)string.str);
}

int
main(int argc, char **argv) {
  thread_ctx.log_proc = windows_log;
  os.window_size.x = 1280;
  os.window_size.y = 720;
  os.window_resizable = 1;
  assert(os_init());
  Font font = os_load_font(os.perm_arena, 72, "Arial", 0);

  f22 = load_obj_dump(os.perm_arena, "plane.bin"_s);
  sponza = load_obj_dump(os.perm_arena, "sponza.bin"_s);
  scene_callback();

  int screen_x = 1280;
  int screen_y = 720;

  r.camera_pos = {0,0,-2};
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
    frame_data = string_fmt(os.frame_arena, "FPS:%f dt:%f frame:%u", os.fps, os.delta_time, os.frame);
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
