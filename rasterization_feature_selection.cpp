struct Token {
  char* s;
  int len;
};

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
  if (a.len != len) return false;
  for (int i = 0; i < len; i++) {
    if (a.s[i] != b[i]) return false;
  }
  return true;
}

bool combinations[64][8] = { { 0, 0 }, { 1, 1 }, { 0, 1 }, { 1, 0 } };
int combinations_to_make = 4;

struct Lexer {
  char* stream;
  char* data;
  char output[10000];

  Token next() {
    while (is_whitespace(*stream)) stream++;
    Token result = {};
    result.s = stream++;
    if (is_alphabetic(*result.s)) {
      while (is_alphabetic(*stream) || *stream == '_') stream++;
    }
    result.len = (int)(stream - result.s);
    return result;
  }

  bool match(const char* str, Token* out = 0) {
    Lexer l = *this;
    Token token = next();
    if (out) *out = token;
    if (equals(token, str)) {
      return true;
    }
    *this = l;
    return false;
  }
};

struct Section {
  char* begin, * end;
  char* name, * name_end;
  int if_clause_i;
  bool is_else;
};

FUNCTION void generate_stuff() {
  char* data = os.read_file("main.cpp");
  FILE* f = fopen("raster_functions.cpp", "w");
  ASSERT(f);
  Section sections[100] = {};
  int sections_count = 0;
  int if_clause_count = 0;
  Lexer lexer = { data, data };
  for (;;) {
    Token token = lexer.next();
    if (*token.s == 0) break;

    if (equals(token, "FUNCTION") &&
      lexer.match("void") &&
      lexer.match("draw_triangle", &token)) {
      Section* section = sections + sections_count++;
      section->begin = token.s + token.len;
      int indent = 1;
      while (lexer.next().s[0] != '{');
      for (; ; ) {
        token = lexer.next();
        if (token.s[0] == '{') indent++;
        else if (token.s[0] == '}') indent--;
        else if (token.s[0] == '#') {
          if (lexer.match("if")) {
            section->end = token.s;

            if_clause_count++;
            token = lexer.next();
            section = sections + sections_count++;
            section->if_clause_i = if_clause_count;
            section->is_else = false;
            section->name = token.s;
            section->name_end = token.s + token.len;
            section->begin = section->name_end;
          }
          else if (lexer.match("else")) {
            section->end = token.s;
            char* name = section->name;
            char* name_end = section->name_end;

            section = sections + sections_count++;
            section->is_else = true;
            section->if_clause_i = if_clause_count;
            section->name = name;
            section->name_end = name_end;
            section->begin = token.s + 5;
          }
          else if (lexer.match("endif")) {
            section->end = token.s;
            section = sections + sections_count++;
            section->begin = token.s + 6;
          }
        }

        if (indent == 0) {
          section->end = token.s + 1;
          break;
        }
      }

      for (int ci = 0; ci < combinations_to_make; ci++) {
        // @Note: Figure out function name
        fprintf(f, "\nFUNCTION void draw_triangle");
        for (int i = 0; i < sections_count; i++) {
          section = sections + i;
          if (section->name && section->is_else == !combinations[ci][section->if_clause_i - 1]) {
            fprintf(f, "_%.*s_", (int)(section->name_end - section->name), section->name);
            fprintf(f, "%s", section->is_else ? "off" : "on");
          }
        }
        // @Note: Figure out function content
        for (int i = 0; i < sections_count; i++) {
          section = sections + i;
          if (!section->name) {
            fprintf(f, "%.*s %.*s", (int)(section->name_end - section->name), section->name, (int)(section->end - section->begin), section->begin);
          }
          else {
            if (section->is_else == !combinations[ci][section->if_clause_i - 1]) {
              fprintf(f, "// %.*s %.*s", (int)(section->name_end - section->name), section->name, (int)(section->end - section->begin), section->begin);
            }
          }
        }
      }




      fclose(f);
    }
  }
}