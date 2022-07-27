
function U8
to_lower_case(U8 a) {
  if (a >= 'A' && a <= 'Z')
    a += 32;
  return a;
}

function U8
to_upper_case(U8 a) {
  if (a >= 'a' && a <= 'z')
    a -= 32;
  return a;
}

function U8
char_to_lower(U8 c){
  if(c >= 'A' && c <= 'Z')
    c += 32;
  return c;
}

function U8
char_to_upper(U8 c){
  if(c >= 'a' && c <= 'z')
    c -= 32;
  return c;
}

function B32
is_whitespace(U8 w) {
  bool result = w == '\n' || w == ' ' || w == '\t' || w == '\v' || w == '\r';
  return result;
}

function B32
is_alphabetic(U8 a) {
  if ((a >= 'a' && a <= 'z') || (a >= 'A' && a <= 'Z')) {
    return true;
  }
  return false;
}

function B32
is_number(U8 a) {
  B32 result = a >= '0' && a <= '9';
  return result;
}

function B32
is_alphanumeric(U8 a) {
  B32 result = is_number(a) || is_alphabetic(a);
  return result;
}

function B32
string_compare(String a, String b, B32 ignore_case = false) {
  if (a.len != b.len)
    return false;
  for (S64 i = 0; i < a.len; i++) {
    U8 A = a.str[i];
    U8 B = b.str[i];
    if (ignore_case) {
      A = to_lower_case(A);
      B = to_lower_case(B);
    }
    if (A != B)
      return false;
  }
  return true;
}

function B32
operator==(String a, String b){
  return string_compare(a,b);
}

function String
string_copy(Allocator *a, String string){
  U8 *copy = exp_alloc_array(a, U8, string.len+1);
  memory_copy(copy, string.str, string.len);
  copy[string.len] = 0;
  return String{copy, string.len};
}

function String
string_fmtv(Allocator *a, const char *str, va_list args1) {
  va_list args2;
  va_copy(args2, args1);
  S64 len = stbsp_vsnprintf(0, 0, str, args2);
  va_end(args2);

  char *result = exp_alloc_array(a, char, len + 1);
  stbsp_vsnprintf(result, len + 1, str, args1);

  String res = {(U8 *)result, len};
  return res;
}

#define STRING_FMT(alloc, str, result)              \
va_list args1;                                    \
va_start(args1, str);                             \
String result = string_fmtv(alloc, str, args1);   \
va_end(args1)

function String
string_fmt(Allocator *a, const char *str, ...) {
  STRING_FMT(a, str, result);
  return result;
}

//-----------------------------------------------------------------------------
// String builder
//-----------------------------------------------------------------------------
struct String_Builder_Block{
  String_Builder_Block *next;
  S64 cap;
  S64 len;
  U8 data[0];
};

struct String_Builder{
  Allocator *allocator;
  String_Builder_Block *first_free;
  String_Builder_Block *first;
  String_Builder_Block *last;
  U64 di;

  void reset(){
    for(;;){
      auto *block = first;
      first = first->next;

      block->next = first_free;
      first_free  = block;

      if(!first) break;
    }

    last = 0;
    assert(!last && !first);
  }

  void push_block(SizeU size){
    String_Builder_Block *block = 0;
    if(first_free){
      block = first_free;
      first_free = first_free->next;
    } else{
      block = (String_Builder_Block *)exp_alloc(allocator, sizeof(String_Builder_Block) + size);
    }
    memory_zero(block, sizeof(String_Builder_Block)+1); // Also clear first byte of character data
    block->cap = size;
    SLLQueuePush(first, last, block);
  }

  void init(S64 size = 4096){
    assert(allocator);
    push_block(size);
  }

  void append_data(void *data, S64 size){
    if(first == 0){
      init();
    }

    S64 remaining_cap = last->cap - last->len;
    if(size > remaining_cap){
      S64 new_block_size = max(last->cap*2, size*2);
      push_block(new_block_size);
    }

    U8 *write_address = last->data + last->len;
    last->len += size;
    memory_copy(write_address, data, size);
  }

  void addf(const char *str, ...){
    if(first == 0){
      init();
    }
    va_list args, args2;
    va_start(args, str);
    retry:{
      String_Builder_Block *block = last;
      S64 block_size = block->cap - block->len;
      char *write_address = (char *)block->data + block->len;

      va_copy(args2, args);
      int written = stbsp_vsnprintf(write_address, block_size, str, args2);
      va_end(args2);

      if(written > (block_size-1)){
        S64 new_block_size = max(4096, (written+1)*2);
        push_block(new_block_size);
        goto retry;
      }
      block->len += written;
    }
    va_end(args);
    di++;
  }
};

function String_Builder
string_builder_make(Allocator *a, S64 first_block_size = 4096){
  String_Builder sb = {a};
  sb.init(first_block_size);
  return sb;
}

enum String_Builder_Flag{
  String_Builder_Flag_None = 0,
  String_Builder_Flag_AddSize = 0,
};

function String
string_flatten(Allocator *a, String_Builder *b, String_Builder_Flag flags = String_Builder_Flag_None){
  // @Note(Krzosa): Compute size to allocate
  S64 size = 1;
  if(is_flag_set(flags, String_Builder_Flag_AddSize)) size += sizeof(SizeU);
  For_List(b->first){
    size += it->len;
  }

  String result = {};
  result.str = (U8 *)exp_alloc(a, size);
  if(is_flag_set(flags, String_Builder_Flag_AddSize)) {
    memory_copy(result.str + result.len, &size, sizeof(S64));
    result.len += sizeof(S64);
  }

  // @Note(Krzosa): Copy the content of each block into the string
  For_List(b->first){
    memory_copy(result.str + result.len, it->data, it->len);
    result.len += it->len;
  }

  result.str[result.len] = 0;
  return result;
}

function void
test_string_builder(){
  Scratch scratch;
  String_Builder sb = string_builder_make(scratch, 4);
  sb.addf("Thing, %d", 242252);
  String f = string_flatten(scratch, &sb);
  assert(string_compare(f, "Thing, 242252"_s));
  sb.addf("-%f %f %f", 23.0, 42.29, 2925.2);
  f = string_flatten(scratch, &sb);
  sb.reset();
}

//-----------------------------------------------------------------------------
// String ops
//-----------------------------------------------------------------------------
function void
string_path_normalize(String s) {
  for (S64 i = 0; i < s.len; i++) {
    if (s.str[i] == '\\')
      s.str[i] = '/';
  }
}

function String
string_make(char *str, S64 len) {
  String result;
  result.str = (U8 *)str;
  result.len = len;
  return result;
}

function String
string_make(U8 *str, S64 len) {
  return string_make((char*)str, len);
}

function String
string_chop(String string, S64 len) {
  len = clamp_top(len, string.len);
  String result = string_make(string.str, string.len - len);
  return result;
}

function String
string_skip(String string, S64 len) {
  len = clamp_top(len, string.len);
  S64 remain = string.len - len;
  String result = string_make(string.str + len, remain);
  return result;
}

function String
string_get_postfix(String string, S64 len) {
  len = clamp_top(len, string.len);
  S64 remain_len = string.len - len;
  String result = string_make(string.str + remain_len, len);
  return result;
}

function String
string_get_prefix(String string, S64 len) {
  len = clamp_top(len, string.len);
  String result = string_make(string.str, len);
  return result;
}

function String
string_slice(String string, S64 first_index, S64 one_past_last_index) {
  assert_msg(first_index < one_past_last_index, "string_slice, first_index is bigger then one_past_last_index");
  assert_msg(string.len > 0, "Slicing string of length 0! Might be an error!");
  String result = string;
  if (string.len > 0) {
    if (one_past_last_index > first_index) {
      first_index = clamp_top(first_index, string.len - 1);
      one_past_last_index = clamp_top(one_past_last_index, string.len);
      result.str += first_index;
      result.len = one_past_last_index - first_index;
    }
    else {
      result.len = 0;
    }
  }
  return result;
}

function String
string_trim(String string) {
  if (string.len == 0) return string;


  S64 whitespace_begin = 0;
  for (; whitespace_begin < string.len; whitespace_begin++) {
    if (!is_whitespace(string.str[whitespace_begin])) {
      break;
    }
  }

  S64 whitespace_end = string.len;
  for (; whitespace_end != whitespace_begin; whitespace_end--) {
    if (!is_whitespace(string.str[whitespace_end - 1])) {
      break;
    }
  }

  if (whitespace_begin == whitespace_end) {
    string.len = 0;
  }
  else {
    string = string_slice(string, whitespace_begin, whitespace_end);
  }

  return string;
}

function String
string_trim_end(String string) {
  S64 whitespace_end = string.len;
  for (; whitespace_end != 0; whitespace_end--) {
    if (!is_whitespace(string.str[whitespace_end - 1])) {
      break;
    }
  }

  String result = string_get_prefix(string, whitespace_end);
  return result;
}

function String
string_to_lower_case(Allocator *arena, String s) {
  String copy = string_copy(arena, s);
  for (U64 i = 0; i < copy.len; i++) {
    copy.str[i] = to_lower_case(copy.str[i]);
  }
  return copy;
}

function String
string_to_upper_case(Allocator *arena, String s) {
  String copy = string_copy(arena, s);
  for (U64 i = 0; i < copy.len; i++) {
    copy.str[i] = to_upper_case(copy.str[i]);
  }
  return copy;
}

FLAG32(MatchFlag){
  MatchFlag_None=0,
  MatchFlag_FindLast=1,
  MatchFlag_IgnoreCase=2,
};

function B32
string_find(String string, String find, MatchFlag flags, S64 *index_out) {
  B32 result = false;
  if (flags & MatchFlag_FindLast) {
    for (S64 i = string.len; i != 0; i--) {
      S64 index = i - 1;
      String substring = string_slice(string, index, index + find.len);
      if (string_compare(substring, find, flags & MatchFlag_IgnoreCase)) {
        if (index_out)
          *index_out = index;
        result = true;
        break;
      }
    }
  }
  else {
    for (S64 i = 0; i < string.len; i++) {
      String substring = string_slice(string, i, i + find.len);
      if (string_compare(substring, find, flags & MatchFlag_IgnoreCase)) {
        if (index_out)
          *index_out = i;
        result = true;
        break;
      }
    }
  }

  return result;
}

function String
string_chop_last_slash(String s) {
  String result = s;
  string_find(s, "/"_s, MatchFlag_FindLast, &result.len);
  return result;
}

function String
string_chop_last_period(String s) {
  String result = s;
  string_find(s, "."_s, MatchFlag_FindLast, &result.len);
  return result;
}

function String
string_skip_to_last_slash(String s) {
  S64 pos;
  String result = s;
  if (string_find(s, "/"_s, MatchFlag_FindLast, &pos)) {
    result = string_skip(result, pos + 1);
  }
  return result;
}

function String
string_skip_to_last_period(String s) {
  S64 pos;
  String result = s;
  if (string_find(s, "."_s, MatchFlag_FindLast, &pos)) {
    result = string_skip(result, pos + 1);
  }
  return result;
}

function S64
string_len(char *string){
  S64 len = 0;
  while(*string++!=0)len++;
  return len;
}

function String
string_from_cstring(char *string){
  String result;
  result.str = (U8 *)string;
  result.len = string_len(string);
  return result;
}





