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

struct Obj {
  DynamicArray<Vec3> vertices;
  DynamicArray<Vec2> texture_coordinates;
  DynamicArray<Vec3> normals;
  DynamicArray<ObjMesh> mesh;
};

namespace obj {
  enum class TokenType {
    none, word, number, whitespace, end
  };

  struct Token {
    TokenType type;
    double number;
    union {
      struct {
        char* s;
        int len;
      };
      S8 s8;
    };
  };

  FN Token next_token_raw(char** data) {
    Token result = {};
    result.s = *data;
    *data += 1;

    if (is_alphabetic(*result.s)) {
      result.type = TokenType::word;
      while (!is_whitespace(**data)) {
        *data += 1;
      }
      result.len = (int)(*data - result.s);
    }
    else if (is_number(*result.s) || *result.s == '-') {
      result.type = TokenType::number;
      while (is_number(**data) || **data == '.') {
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
      result.type = TokenType::whitespace;
      while (is_whitespace(**data)) *data += 1;
      result.len = (int)(*data - result.s);
    }
    else if (*result.s == 0) {
      result.type = TokenType::end;
    }
    else if (*result.s >= '!') {
      result.type = (TokenType)*result.s;
    }

    return result;
  }

  FN Token next_token(char** data) {
    Token result;
    do {
      result = next_token_raw(data);
    } while (result.type == TokenType::whitespace);
    return result;
  }

  FN double expect_number(char** data) {
    Token t = next_token(data);
    TASSERT(t.type == TokenType::number); // @Todo: Error handling, error flag
    return t.number;
  }

  FN void expect_token(char** data, char token) {
    Token t = next_token(data);
    TASSERT(t.type == (TokenType)token); // @Todo: Error handling, error flag
  }

  FN void debug_expect_raw(char** data, TokenType type) {
    char* data_temp = *data;
    TASSERT(next_token_raw(&data_temp).type == type);
  }

  // Each face needs to have own material_id, group_id, smoothing ..
  Obj parse(char* data) {
    Obj result = {};
    int smoothing = 0;
    ObjMesh *mesh = 0;
    int material_id = -1;

    for (;; ) {
      Token token = next_token(&data);
      if (token.type == TokenType::end) break;
      else if (token.type == TokenType::word) {
        if (string_compare(token.s8, LIT("v"))) {
          Vec3 *vertex = result.vertices.push_empty();
          vertex->x = (float)expect_number(&data);
          vertex->y = (float)expect_number(&data);
          vertex->z = (float)expect_number(&data);
          debug_expect_raw(&data, TokenType::whitespace);
        }
        else if (string_compare(token.s8, LIT("vt"))) {
          Vec2 *tex = result.texture_coordinates.push_empty();
          tex->x = (float)expect_number(&data);
          tex->y = (float)expect_number(&data);
          debug_expect_raw(&data, TokenType::whitespace);
        }
        else if (string_compare(token.s8, LIT("vn"))) {
          Vec3 *norm = result.normals.push_empty();
          norm->x = (float)expect_number(&data);
          norm->y = (float)expect_number(&data);
          norm->z = (float)expect_number(&data);
          debug_expect_raw(&data, TokenType::whitespace);
        }
        else if (string_compare(token.s8, LIT("mtlib"))) {
          Token t = next_token(&data);
          TASSERT(t.type == TokenType::word);
        }
        else if (string_compare(token.s8, LIT("usemtl"))) {
          Token t = next_token(&data);
          TASSERT(t.type == TokenType::word);
        }
        else if (string_compare(token.s8, LIT("o"))) {
          Token t = next_token(&data);
          TASSERT(t.type == TokenType::word);
          mesh = result.mesh.push_empty();
          ZERO_STRUCT(mesh);
          U64 len = CLAMP_TOP(t.len, 64);
          memory_copy(t.s, mesh->name, len);
        }
        else if (string_compare(token.s8, LIT("s"))) {
          Token t = next_token(&data);
          if (t.type == TokenType::number) {
            smoothing = (int)t.number;
          }
          else {
            TASSERT(t.type == TokenType::word);
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
          Token t = next_token(&data);
          TASSERT(t.type == TokenType::word);
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
          //debug_expect_raw(&data, TokenType::whitespace);
        }
      }
    }
    return result;
  }

  FN void test_lex() {
    const char* d = "v 0.885739 0.001910 -0.380334";
    char* dd = (char *)d;
    TASSERT(next_token(&dd).type == TokenType::word);
    Token t = next_token(&dd); TASSERT(t.type == TokenType::number && t.number > 0.8857);
    t = next_token(&dd); TASSERT(t.type == TokenType::number && t.number > 0.0019);
    t = next_token(&dd); TASSERT(t.type == TokenType::number && t.number < -0.38);
    d = "# Blender v2.79 (sub 0) OBJ File: 'fighters_0.blend'\n"
      "# www.blender.org\n"
      "mtllib f-22.mtl\n"
      "o F-22\n";
    dd = (char *)d;
    t = next_token(&dd); TASSERT(t.type == TokenType::word && string_compare(t.s8, LIT("mtllib")));
    t = next_token(&dd); TASSERT(t.type == TokenType::word && string_compare(t.s8, LIT("f-22.mtl")));
    t = next_token(&dd); TASSERT(t.type == TokenType::word && string_compare(t.s8, LIT("o")));
    t = next_token(&dd); TASSERT(t.type == TokenType::word && string_compare(t.s8, LIT("F-22")));
  }

  void test() {
    test_lex();
  }
}

FUNCTION
Obj load_obj(S8 file) {
  Scratch scratch;
  S8 data = os_read_file(scratch, file).error_is_fatal();
  PUSH_SIZE(scratch, 1);
  Obj result = obj::parse((char *)data.str);
  return result;
}
