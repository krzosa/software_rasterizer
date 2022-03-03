template<class T>
struct DynamicArray {
  T *e;
  U64 cap, len;
  T *push_empty(int element_count = 1) {
    if (cap == 0) {
      cap = 8;
      e = (T*)malloc(sizeof(T)*cap);
    }
    else if (len + element_count > cap) {
      U64 new_size = (len + element_count) * 2;
      void *ptr = realloc(e, new_size*sizeof(T));
      if (!ptr) FATAL_ERROR("Ran out of memory! Cant allocate more.");
      e = (T *)ptr;
      cap = new_size;
    }

    T *result = e + len;
    len += element_count;
    return result;
  }
  void push(T element) {
    T *result = push_empty();
    *result = element;
  }
};

struct ObjIndex {
  int vertex[3];
  int tex[3];
  int normal[3];
  I32 material_id;
  I32 smoothing_group_id;
};

struct ObjMesh {
  char name[64];
  DynamicArray<ObjIndex> indices;
};

struct OBJMaterial {
  char name[64];
  U32  name_len;
  Bitmap texture_ambient; // map_Ka
  Bitmap texture_diffuse; // map_Kd
  Bitmap texture_dissolve; // map_d
  Bitmap texture_displacment; // map_Disp
  F32 non_transparency; // d
  F32 transparency; // Tr
  F32 optical_density; // Ni
  F32 shininess; // Ns
  I32 illumination_model; // illum 
  Vec3 ambient_color; // Ka
  Vec3 diffuse_color; // Kd
  Vec3 specular_color; // Ks
};

struct Obj {
  S8 name;
  DynamicArray<Vec3> vertices;
  DynamicArray<Vec2> texture_coordinates;
  DynamicArray<Vec3> normals;
  DynamicArray<ObjMesh> mesh;
  DynamicArray<OBJMaterial> materials;
};

FUNCTION
Bitmap load_image(const char* path) {
  int x, y, n;
  unsigned char* data = stbi_load(path, &x, &y, &n, 4);
  Bitmap result = { (U32*)data, x, y };
#if PREMULTIPLIED_ALPHA_BLENDING
  if(data) {
    U32 *p = result.pixels;
    for (int Y = 0; Y < y; Y++) {
      for (int X = 0; X < x; X++) {
        Vec4 color = vec4abgr(*p);
        color.r *= color.a;
        color.g *= color.a;
        color.b *= color.a;
        *p++ = vec4_to_u32abgr(color);
      }
    }  
  }
#endif
  return result;
}

enum class OBJTokenType {
  none, word, number, whitespace, end
};

struct OBJToken {
  OBJTokenType type;
  double number;
  union {
    struct {
      char* s;
      int len;
    };
    S8 s8;
  };
};

FUNCTION OBJToken next_token_raw(char** data) {
  OBJToken result = {};
  result.s = *data;
  *data += 1;

  if (is_alphabetic(*result.s)) {
    result.type = OBJTokenType::word;
    while (!is_whitespace(**data)) {
      *data += 1;
    }
    result.len = (int)(*data - result.s);
  }
  else if (is_number(*result.s) || *result.s == '-') {
    result.type = OBJTokenType::number;
    while (is_number(**data) || **data == '.' ||  **data == 'e' || **data == '-') {
      *data += 1;
    }
    result.number = atof(result.s);
    result.len = (int)(*data - result.s);
  }
  else if (*result.s == '#') {
    while (**data != '\n') *data += 1;
    result = next_token_raw(data);
  }
  else if (is_whitespace(*result.s)) {
    result.type = OBJTokenType::whitespace;
    while (is_whitespace(**data)) *data += 1;
    result.len = (int)(*data - result.s);
  }
  else if (*result.s == 0) {
    result.type = OBJTokenType::end;
  }
  else if (*result.s >= '!') {
    result.type = (OBJTokenType)*result.s;
  }

  return result;
}

FUNCTION OBJToken next_token(char** data) {
  OBJToken result;
  do {
    result = next_token_raw(data);
  } while (result.type == OBJTokenType::whitespace);
  return result;
}

FUNCTION double expect_number(char** data) {
  OBJToken t = next_token(data);
  TASSERT(t.type == OBJTokenType::number); // @Todo: Error handling, error flag
  return t.number;
}

FUNCTION void expect_token(char** data, char token) {
  OBJToken t = next_token(data);
  TASSERT(t.type == (OBJTokenType)token); // @Todo: Error handling, error flag
}

FUNCTION void debug_expect_raw(char** data, OBJTokenType type) {
  char* data_temp = *data;
  OBJToken t = next_token_raw(&data_temp);
  TASSERT(t.type == type);
}

FUNCTION void parse_mtl(Arena *arena, Obj* obj, S8 path_obj_folder, S8 mtl_file) {
  Scratch scratch;
  char *data = (char *)mtl_file.str;
  OBJMaterial *m = 0;
  for (;;) {
    OBJToken token = next_token(&data);
    if (token.type == OBJTokenType::end) break;
    else if (token.type == OBJTokenType::word) {
      if (string_compare(token.s8, LIT("newmtl"))) {
        token = next_token(&data);
        m = obj->materials.push_empty();
        ZERO_STRUCT(m);
        m->name_len = CLAMP_TOP(token.len, 64);
        memory_copy(token.s8.str, m->name, m->name_len);
      }
      else if (string_compare(token.s8, LIT("Ns"))) {
        m->shininess = expect_number(&data);
      }
      else if (string_compare(token.s8, LIT("Ka"))) {
        m->ambient_color.x = expect_number(&data);
        m->ambient_color.y = expect_number(&data);
        m->ambient_color.z = expect_number(&data);
      }
      else if (string_compare(token.s8, LIT("Kd"))) {
        m->diffuse_color.x = expect_number(&data);
        m->diffuse_color.y = expect_number(&data);
        m->diffuse_color.z = expect_number(&data);
      }
      else if (string_compare(token.s8, LIT("Ks"))) {
        m->specular_color.x = expect_number(&data);
        m->specular_color.y = expect_number(&data);
        m->specular_color.z = expect_number(&data);
      }
      else if (string_compare(token.s8, LIT("Ni"))) {
        m->optical_density = expect_number(&data);
      }
      else if (string_compare(token.s8, LIT("d"))) {
        m->non_transparency = expect_number(&data);
      }
      else if (string_compare(token.s8, LIT("illum"))) {
        m->illumination_model = (I32)expect_number(&data);
      }
      else if (string_compare(token.s8, LIT("map_Kd"))) {
        OBJToken t = next_token(&data);
        S8 path = string_format(scratch, "%s/%s\0", path_obj_folder, t.s8);
        m->texture_diffuse = load_image((const char *)path.str);
      }
      else if (string_compare(token.s8, LIT("map_Ka"))) {
        OBJToken t = next_token(&data);
        S8 path = string_format(scratch, "%s/%s\0", path_obj_folder, t.s8);
        m->texture_ambient = load_image((const char *)path.str);
      }
      else if (string_compare(token.s8, LIT("map_d"))) {
        OBJToken t = next_token(&data);
        S8 path = string_format(scratch, "%s/%s\0", path_obj_folder, t.s8);
        m->texture_dissolve = load_image((const char *)path.str);
      }
      else if (string_compare(token.s8, LIT("map_Disp"))) {
        OBJToken t = next_token(&data);
        S8 path = string_format(scratch, "%s/%s\0", path_obj_folder, t.s8);
        m->texture_displacment = load_image((const char *)path.str);
      }
    }
  }
}

FUNCTION Obj parse(Arena *arena, char* data, S8 path_obj_folder) {
  Scratch mtl_scratch;
  Obj result = {};
  int smoothing = 0;
  ObjMesh *mesh = result.mesh.push_empty();
  ZERO_STRUCT(mesh);
  int material_id = -1;

  for (;; ) {
    OBJToken token = next_token(&data);
    if (token.type == OBJTokenType::end) break;
    else if (token.type == OBJTokenType::word) {
      if (string_compare(token.s8, LIT("v"))) {
        Vec3 *vertex = result.vertices.push_empty();
        vertex->x = (float)expect_number(&data);
        vertex->y = (float)expect_number(&data);
        vertex->z = (float)expect_number(&data);
        debug_expect_raw(&data, OBJTokenType::whitespace);
      }
      else if (string_compare(token.s8, LIT("vt"))) {
        Vec2 *tex = result.texture_coordinates.push_empty();
        tex->x = (float)expect_number(&data);
        tex->y = (float)expect_number(&data);
        debug_expect_raw(&data, OBJTokenType::whitespace);
      }
      else if (string_compare(token.s8, LIT("vn"))) {
        Vec3 *norm = result.normals.push_empty();
        norm->x = (float)expect_number(&data);
        norm->y = (float)expect_number(&data);
        norm->z = (float)expect_number(&data);
        debug_expect_raw(&data, OBJTokenType::whitespace);
      }
      else if (string_compare(token.s8, LIT("mtllib"))) {
        OBJToken t = next_token(&data);
        S8 path = string_format(mtl_scratch, "%s/%s", path_obj_folder, t.s8);
        Result<S8> mtl_file = os_read_file(mtl_scratch, path);
        if(mtl_file.no_error()) {
          PUSH_SIZE(mtl_scratch, 1);
          parse_mtl(arena, &result, path_obj_folder, mtl_file.result);  
        }
      }
      else if (string_compare(token.s8, LIT("usemtl"))) {
        OBJToken t = next_token(&data);
        TASSERT(t.type == OBJTokenType::word);
        for(U64 i = 0; i < result.materials.len; i++) {
          OBJMaterial *m = result.materials.e + i;
          if(string_compare(string_make((U8 *)m->name, m->name_len), t.s8)) {
            material_id = i;
            break;
          }
        }
      }
      else if (string_compare(token.s8, LIT("o"))) {
        OBJToken t = next_token(&data);
        TASSERT(t.type == OBJTokenType::word);
        if (mesh->indices.len != 0) {
          mesh = result.mesh.push_empty();
          ZERO_STRUCT(mesh);
        }
        else {
          U64 len = CLAMP_TOP(t.len, 64);
          memory_copy(t.s, mesh->name, len);
        }
      }
      else if (string_compare(token.s8, LIT("s"))) {
        OBJToken t = next_token(&data);
        if (t.type == OBJTokenType::number) {
          smoothing = (int)t.number;
        }
        else {
          TASSERT(t.type == OBJTokenType::word);
          if (string_compare(t.s8, LIT("on"))) {
            smoothing = 1;
          }
          else if (string_compare(t.s8, LIT("off"))) {
            smoothing = 0;
          }
          else INVALID_CODEPATH;
        }
        
      }
      else if (string_compare(token.s8, LIT("g"))) {
        OBJToken t = next_token(&data);
        TASSERT(t.type == OBJTokenType::word);
      }
      else if (string_compare(token.s8, LIT("f"))) {
        ObjIndex *i = mesh->indices.push_empty();
        i->smoothing_group_id = smoothing;
        i->material_id = material_id;
        i->vertex[0] = (int)expect_number(&data);
        expect_token(&data, '/');
        i->tex[0] = (int)expect_number(&data);
        expect_token(&data, '/');
        i->normal[0] = (int)expect_number(&data);

        i->vertex[1] = (int)expect_number(&data);
        expect_token(&data, '/');
        i->tex[1] = (int)expect_number(&data);
        expect_token(&data, '/');
        i->normal[1] = (int)expect_number(&data);

        i->vertex[2] = (int)expect_number(&data);
        expect_token(&data, '/');
        i->tex[2] = (int)expect_number(&data);
        expect_token(&data, '/');
        i->normal[2] = (int)expect_number(&data);
        //debug_expect_raw(&data, OBJTokenType::whitespace);
      }
    }
  }
  return result;
}

FUNCTION void test_lex() {
  const char* d = "v 0.885739 0.001910 -0.380334";
  char* dd = (char *)d;
  TASSERT(next_token(&dd).type == OBJTokenType::word);
  OBJToken t = next_token(&dd); TASSERT(t.type == OBJTokenType::number && t.number > 0.8857);
  t = next_token(&dd); TASSERT(t.type == OBJTokenType::number && t.number > 0.0019);
  t = next_token(&dd); TASSERT(t.type == OBJTokenType::number && t.number < -0.38);
  d = "# Blender v2.79 (sub 0) OBJ File: 'fighters_0.blend'\n"
    "# www.blender.org\n"
    "mtllib f-22.mtl\n"
    "o F-22\n";
  dd = (char *)d;
  t = next_token(&dd); TASSERT(t.type == OBJTokenType::word && string_compare(t.s8, LIT("mtllib")));
  t = next_token(&dd); TASSERT(t.type == OBJTokenType::word && string_compare(t.s8, LIT("f-22.mtl")));
  t = next_token(&dd); TASSERT(t.type == OBJTokenType::word && string_compare(t.s8, LIT("o")));
  t = next_token(&dd); TASSERT(t.type == OBJTokenType::word && string_compare(t.s8, LIT("F-22")));
}

void test() {
  test_lex();
}

FUNCTION Obj load_obj(Arena *arena, S8 file) {
  Scratch scratch;
  S8 data = os_read_file(scratch, file).error_is_fatal();
  PUSH_SIZE(scratch, 1);
  S8 path = string_chop_last_slash(file);
  Obj result = parse(arena, (char *)data.str, path);
  result.name = file;
  return result;
}
