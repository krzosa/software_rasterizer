#include "objparser.h"
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

enum class TokenType {
  none, word, number, whitespace, end
};

struct Token {
  TokenType type;
  union {
    struct {
      char* s;
      int   len;
    };
    double number;
  };
};

static
bool Obj_IsAlphabetic(char w) {
  bool result = (w >= 'a' && w <= 'z') || (w >= 'A' && w <= 'Z');
  return result;
}

static
bool Obj_IsNumber(char w) {
  bool result = w >= '0' && w <= '9';
  return result;
}

static 
bool Obj_IsWhitespace(char w) {
  bool result = w == '\n' || w == ' ' || w == '\t' || w == '\v' || w == '\r';
  return result;
}

static
int Obj_StringLen(char* a) {
  int result = 0;
  while (*a++ != 0) result++;
  return result;
}

static
bool Obj_Equals(Token a, const char* b) {
  int len = Obj_StringLen((char*)b);
  if (a.type != TokenType::word) return false;
  if (a.len != len) return false;
  for (int i = 0; i < len; i++) {
    if (a.s[i] != b[i]) return false;
  }
  return true;
}

static
Token Obj_NextTokenRaw(char** data) {
  Token result = {};
  result.s = *data;
  *data += 1;

  if (Obj_IsAlphabetic(*result.s)) {
    result.type = TokenType::word;
    while (!Obj_IsWhitespace(**data)) {
      *data+=1;
    }
    result.len = (int)(*data - result.s);
  }
  else if (Obj_IsNumber(*result.s) || *result.s == '-') {
    result.type = TokenType::number;
    while (Obj_IsNumber(**data) || **data == '.') {
      *data += 1;
    }
    result.number = atof(result.s);
  }
  else if (*result.s == '#') {
    while (**data != '\n') *data += 1;
    result = Obj_NextTokenRaw(data);
  }
  else if (Obj_IsWhitespace(*result.s)) {
    result.type = TokenType::whitespace;
    while (Obj_IsWhitespace(**data)) *data += 1;
    result.len = (int)(*data - result.s);
  }
  else if (*result.s == 0) {
    result.type = TokenType::end;
  }
  else if(*result.s >= '!') {
    result.type = (TokenType)*result.s;
  }

  return result;
}

static
Token Obj_NextToken(char** data) {
  Token result;
  do {
    result = Obj_NextTokenRaw(data);
  } while (result.type == TokenType::whitespace);
  return result;
}

static
double Obj_ExpectNumber(char** data) {
  Token t = Obj_NextToken(data);
  assert(t.type == TokenType::number); // @Todo: Error handling, error flag
  return t.number;
}

static
void Obj_ExpectToken(char** data, char token) {
  Token t = Obj_NextToken(data);
  assert(t.type == (TokenType)token); // @Todo: Error handling, error flag
}

static
void Obj_Debug_ExpectRaw(char** data, TokenType type) {
  char* data_temp = *data;
  assert(Obj_NextTokenRaw(&data_temp).type == type);
}

struct Obj_Arena {
  char* base;
  size_t size;
  size_t p;
};

static 
char* Obj_Push(Obj_Arena *a, size_t size) {
  char* ptr = a->base;
  if (a->p + size < a->size) {
    ptr += a->p;
    a->p += size;
  }
  else {
    assert(!"Buffer is too small to hold the data!");
  }
  return ptr;
}

Obj Obj_Parse(char* memory, size_t memory_size, char* data) {
  Obj_Arena arena = { memory, memory_size };
  Obj result = {};
  int parsing_vertices = 0;
  int parsing_normals = 0;
  int parsing_textures = 0;

  for (;;) {
    Token token = Obj_NextToken(&data);
    if (token.type == TokenType::end) break;
    else if (token.type == TokenType::word) {
      if (Obj_Equals(token, "v")) {
        assert(parsing_vertices != 2);
        parsing_vertices = 1;
        float* ptr = (float*)Obj_Push(&arena, sizeof(float) * 3);
        ptr[0] = (float)Obj_ExpectNumber(&data);
        ptr[1] = (float)Obj_ExpectNumber(&data);
        ptr[2] = (float)Obj_ExpectNumber(&data);
        if (result.vertices == 0) result.vertices = ptr;
        result.vertices_count++;
        Obj_Debug_ExpectRaw(&data, TokenType::whitespace);
      }
      else if (Obj_Equals(token, "vt")) {
        assert(parsing_textures != 2);
        parsing_textures = 1;
        parsing_vertices = 2;

        float* ptr = (float*)Obj_Push(&arena, sizeof(float) * 2);
        ptr[0] = (float)Obj_ExpectNumber(&data);
        ptr[1] = (float)Obj_ExpectNumber(&data);
        if (result.texture == 0) result.texture = ptr;
        Obj_Debug_ExpectRaw(&data, TokenType::whitespace);
      }
      else if (Obj_Equals(token, "vn")) {
        assert((parsing_textures == 1 || parsing_textures == 2) && parsing_vertices == 2);
        parsing_textures = 2;
        parsing_normals = 1;

        float* ptr = (float*)Obj_Push(&arena, sizeof(float) * 3);
        ptr[0] = (float)Obj_ExpectNumber(&data);
        ptr[1] = (float)Obj_ExpectNumber(&data);
        ptr[2] = (float)Obj_ExpectNumber(&data);
        if (result.normals == 0) result.normals = ptr;
        Obj_Debug_ExpectRaw(&data, TokenType::whitespace);
      }
      else if (Obj_Equals(token, "f")) {
        assert(parsing_normals == 1 && parsing_textures == 2 && parsing_vertices == 2);
        int* ptr = (int*)Obj_Push(&arena, sizeof(int) * 9);
        ptr[0] = (int)Obj_ExpectNumber(&data);
        Obj_ExpectToken(&data, '/');
        ptr[3] = (int)Obj_ExpectNumber(&data);
        Obj_ExpectToken(&data, '/');
        ptr[6] = (int)Obj_ExpectNumber(&data);

        ptr[1] = (int)Obj_ExpectNumber(&data);
        Obj_ExpectToken(&data, '/');
        ptr[4] = (int)Obj_ExpectNumber(&data);
        Obj_ExpectToken(&data, '/');
        ptr[7] = (int)Obj_ExpectNumber(&data);

        ptr[2] = (int)Obj_ExpectNumber(&data);
        Obj_ExpectToken(&data, '/');
        ptr[5] = (int)Obj_ExpectNumber(&data);
        Obj_ExpectToken(&data, '/');
        ptr[8] = (int)Obj_ExpectNumber(&data);
        if (result.indices == 0) result.indices = ptr;
        result.indices_count += 1;

        Obj_Debug_ExpectRaw(&data, TokenType::whitespace);
      }
    }
  }
  result.memory_taken = arena.p;
  return result;
}

static
void Obj_TestLex() {
  const char* d = "v 0.885739 0.001910 -0.380334";
  char* dd = (char *)d;
  assert(Obj_NextToken(&dd).type == TokenType::word);
  Token t = Obj_NextToken(&dd); assert(t.type == TokenType::number && t.number > 0.8857);
  t = Obj_NextToken(&dd); assert(t.type == TokenType::number && t.number > 0.0019);
  t = Obj_NextToken(&dd); assert(t.type == TokenType::number && t.number < -0.38);
  d = "# Blender v2.79 (sub 0) OBJ File: 'fighters_0.blend'\n"
    "# www.blender.org\n"
    "mtllib f-22.mtl\n"
    "o F-22\n";
  dd = (char *)d;
  t = Obj_NextToken(&dd); assert(t.type == TokenType::word && Obj_Equals(t, (char*)"mtllib"));
  t = Obj_NextToken(&dd); assert(t.type == TokenType::word && Obj_Equals(t, (char*)"f-22.mtl"));
  t = Obj_NextToken(&dd); assert(t.type == TokenType::word && Obj_Equals(t, (char*)"o"));
  t = Obj_NextToken(&dd); assert(t.type == TokenType::word && Obj_Equals(t, (char*)"F-22"));
}

void Obj_Test() {
  Obj_TestLex();
}