#include "objparser.h"
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#define FUNCTION static

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

struct Obj_Arena {
  char* base;
  size_t size;
  size_t p;
};

namespace obj {
  FUNCTION bool is_alphabetic(char w) {
    bool result = (w >= 'a' && w <= 'z') || (w >= 'A' && w <= 'Z');
    return result;
  }

  FUNCTION bool is_number(char w) {
    bool result = w >= '0' && w <= '9';
    return result;
  }

  FUNCTION bool is_whitespace(char w) {
    bool result = w == '\n' || w == ' ' || w == '\t' || w == '\v' || w == '\r';
    return result;
  }

  FUNCTION int string_len(char* a) {
    int result = 0;
    while (*a++ != 0) result++;
    return result;
  }

  FUNCTION bool equals(Token a, const char* b) {
    int len = string_len((char*)b);
    if (a.type != TokenType::word) return false;
    if (a.len != len) return false;
    for (int i = 0; i < len; i++) {
      if (a.s[i] != b[i]) return false;
    }
    return true;
  }

  FUNCTION Token next_token_raw(char** data) {
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

  FUNCTION Token next_token(char** data) {
    Token result;
    do {
      result = next_token_raw(data);
    } while (result.type == TokenType::whitespace);
    return result;
  }

  FUNCTION double expect_number(char** data) {
    Token t = next_token(data);
    assert(t.type == TokenType::number); // @Todo: Error handling, error flag
    return t.number;
  }

  FUNCTION void expect_token(char** data, char token) {
    Token t = next_token(data);
    assert(t.type == (TokenType)token); // @Todo: Error handling, error flag
  }

  FUNCTION void debug_expect_raw(char** data, TokenType type) {
    char* data_temp = *data;
    assert(next_token_raw(&data_temp).type == type);
  }

  FUNCTION char* push(Obj_Arena *a, size_t size) {
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

  Obj parse(char* memory, size_t memory_size, char* data) {
    Obj_Arena arena = { memory, memory_size };
    Obj result = {};

    for (; ; ) {
      Token token = next_token(&data);
      if (token.type == TokenType::end) break;
      else if (token.type == TokenType::word) {
        if (equals(token, "v")) {
          float* ptr = (float*)push(&arena, sizeof(float) * 3);
          ptr[0] = (float)expect_number(&data);
          ptr[1] = (float)expect_number(&data);
          ptr[2] = (float)expect_number(&data);
          if (result.vertices == 0) result.vertices = ptr;
          result.vertices_count++;
          debug_expect_raw(&data, TokenType::whitespace);
        }
        else if (equals(token, "vt")) {
          float* ptr = (float*)push(&arena, sizeof(float) * 2);
          ptr[0] = (float)expect_number(&data);
          ptr[1] = (float)expect_number(&data);
          if (result.texture == 0) result.texture = ptr;
          debug_expect_raw(&data, TokenType::whitespace);
        }
        else if (equals(token, "vn")) {
          float* ptr = (float*)push(&arena, sizeof(float) * 3);
          ptr[0] = (float)expect_number(&data);
          ptr[1] = (float)expect_number(&data);
          ptr[2] = (float)expect_number(&data);
          if (result.normals == 0) result.normals = ptr;
          debug_expect_raw(&data, TokenType::whitespace);
        }
        else if (equals(token, "f")) {
          int* ptr = (int*)push(&arena, sizeof(int) * 9);
          ptr[0] = (int)expect_number(&data);
          expect_token(&data, '/');
          ptr[3] = (int)expect_number(&data);
          expect_token(&data, '/');
          ptr[6] = (int)expect_number(&data);

          ptr[1] = (int)expect_number(&data);
          expect_token(&data, '/');
          ptr[4] = (int)expect_number(&data);
          expect_token(&data, '/');
          ptr[7] = (int)expect_number(&data);

          ptr[2] = (int)expect_number(&data);
          expect_token(&data, '/');
          ptr[5] = (int)expect_number(&data);
          expect_token(&data, '/');
          ptr[8] = (int)expect_number(&data);
          if (result.indices == 0) result.indices = ptr;
          result.indices_count += 1;

          debug_expect_raw(&data, TokenType::whitespace);
        }
      }
    }
    result.memory_taken = arena.p;
    return result;
  }

  FUNCTION void test_lex() {
    const char* d = "v 0.885739 0.001910 -0.380334";
    char* dd = (char *)d;
    assert(next_token(&dd).type == TokenType::word);
    Token t = next_token(&dd); assert(t.type == TokenType::number && t.number > 0.8857);
    t = next_token(&dd); assert(t.type == TokenType::number && t.number > 0.0019);
    t = next_token(&dd); assert(t.type == TokenType::number && t.number < -0.38);
    d = "# Blender v2.79 (sub 0) OBJ File: 'fighters_0.blend'\n"
      "# www.blender.org\n"
      "mtllib f-22.mtl\n"
      "o F-22\n";
    dd = (char *)d;
    t = next_token(&dd); assert(t.type == TokenType::word && equals(t, (char*)"mtllib"));
    t = next_token(&dd); assert(t.type == TokenType::word && equals(t, (char*)"f-22.mtl"));
    t = next_token(&dd); assert(t.type == TokenType::word && equals(t, (char*)"o"));
    t = next_token(&dd); assert(t.type == TokenType::word && equals(t, (char*)"F-22"));
  }

  void test() {
    test_lex();
  }
}