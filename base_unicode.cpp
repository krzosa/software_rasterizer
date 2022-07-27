global U32 question_mark32 = '?';
global U16 question_mark16 = 0x003f;
global U8 question_mark8   = '?';

struct String32{
  U32 *str;
  S64 len;
};

struct UTF32_Result{
  U32 out_str;
  S64 advance;
  B32 error;
};

function UTF32_Result
utf8_to_utf32(U8 *c, S64 max_advance) {
  UTF32_Result result = {};

  if ((c[0] & 0b10000000) == 0) { // Check if leftmost zero of first byte is unset
    if(max_advance >= 1){
      result.out_str = c[0];
      result.advance = 1;
    }
    else result.error = 1;
  }

  else if ((c[0] & 0b11100000) == 0b11000000) {
    if ((c[1] & 0b11000000) == 0b10000000) { // Continuation byte required
      if(max_advance >= 2){
        result.out_str = (U32)(c[0] & 0b00011111) << 6u | (c[1] & 0b00111111);
        result.advance = 2;
      }
      else result.error = 2;
    }
    else result.error = 2;
  }

  else if ((c[0] & 0b11110000) == 0b11100000) {
    if ((c[1] & 0b11000000) == 0b10000000 && (c[2] & 0b11000000) == 0b10000000) { // Two continuation bytes required
      if(max_advance >= 3){
        result.out_str = (U32)(c[0] & 0b00001111) << 12u | (U32)(c[1] & 0b00111111) << 6u | (c[2] & 0b00111111);
        result.advance = 3;
      }
      else result.error = 3;
    }
    else result.error = 3;
  }

  else if ((c[0] & 0b11111000) == 0b11110000) {
    if ((c[1] & 0b11000000) == 0b10000000 && (c[2] & 0b11000000) == 0b10000000 && (c[3] & 0b11000000) == 0b10000000) { // Three continuation bytes required
      if(max_advance >= 4){
        result.out_str = (U32)(c[0] & 0b00001111) << 18u | (U32)(c[1] & 0b00111111) << 12u | (U32)(c[2] & 0b00111111) << 6u | (U32)(c[3] & 0b00111111);
        result.advance = 4;
      }
      else result.error = 4;
    }
    else result.error = 4;
  }
  else result.error = 4;

  return result;
}

struct String16{
  U16 *str;
  S64  len;
};

struct UTF16_Result{
  U16 out_str[2];
  S32 len;
  B32 error;
};

function UTF16_Result
utf32_to_utf16(U32 codepoint){
  UTF16_Result result = {};
  if (codepoint < 0x10000) {
    result.out_str[0] = (U16)codepoint;
    result.out_str[1] = 0;
    result.len = 1;
  }
  else if (codepoint <= 0x10FFFF) {
    U32 code = (codepoint - 0x10000);
    result.out_str[0] = (U16)(0xD800 | (code >> 10));
    result.out_str[1] = (U16)(0xDC00 | (code & 0x3FF));
    result.len = 2;
  }
  else{
    result.error = 1;
  }

  return result;
}

struct UTF8_Result{
  U8 out_str[4];
  S32 len;
  B32 error;
};

function UTF8_Result
utf32_to_utf8(U32 codepoint) {
  UTF8_Result result = {};
  if (codepoint <= 0x7F) {
    result.len = 1;
    result.out_str[0] = (U8)codepoint;
  }
  else if (codepoint <= 0x7FF) {
    result.len= 2;
    result.out_str[0] = 0b11000000 | (0b00011111 & (codepoint >> 6));
    result.out_str[1] = 0b10000000 | (0b00111111 & codepoint);
  }
  else if (codepoint <= 0xFFFF) { // 16 bit word
    result.len= 3;
    result.out_str[0] = 0b11100000 | (0b00001111 & (codepoint >> 12)); // 4 bits
    result.out_str[1] = 0b10000000 | (0b00111111 & (codepoint >> 6));  // 6 bits
    result.out_str[2] = 0b10000000 | (0b00111111 & codepoint);         // 6 bits
  }
  else if (codepoint <= 0x10FFFF) {                           // 21 bit word
    result.len= 4;
    result.out_str[0] = 0b11110000 | (0b00000111 & (codepoint >> 18)); // 3 bits
    result.out_str[1] = 0b10000000 | (0b00111111 & (codepoint >> 12)); // 6 bits
    result.out_str[2] = 0b10000000 | (0b00111111 & (codepoint >> 6));  // 6 bits
    result.out_str[3] = 0b10000000 | (0b00111111 & codepoint);         // 6 bits
  }
  else{
    result.error = true;
  }

  return result;
}

function UTF32_Result
utf16_to_utf32(U16 *c, S32 max_advance) {
  UTF32_Result result = {};
  if(max_advance >= 1){
    result.advance = 1;
    result.out_str = c[0];
    if (c[0] >= 0xD800 && c[0] <= 0xDBFF && c[1] >= 0xDC00 && c[1] <= 0xDFFF) {
      if(max_advance >= 2){
        result.out_str = 0x10000;
        result.out_str += (U32)(c[0] & 0x03FF) << 10u | (c[1] & 0x03FF);
        result.advance = 2;
      }
      else result.error = 2;
    }
  }
  else result.error = 1;

  return result;
}

#define unicode_error(question_mark)                                                     \
  {                                                                                      \
    result.str[result.len++] = question_mark;                                            \
    break;                                                                               \
  }

function String32
string16_to_string32(Allocator *allocator, String16 string){
  String32 result = {exp_alloc_array(allocator, U32, string.len+1)};
  for(S64 i = 0; i < string.len;){
    UTF32_Result decode = utf16_to_utf32(string.str + i, string.len - i);
    if(!decode.error){
      i += decode.advance;
      result.str[result.len++] = decode.out_str;
    }
    else unicode_error(question_mark32);
  }

  result.str[result.len] = 0;
  return result;
}

function String32
string8_to_string32(Allocator *allocator, String string){
  String32 result = {exp_alloc_array(allocator, U32, string.len+1)};
  for(S64 i = 0; i < string.len;){
    UTF32_Result decode = utf8_to_utf32(string.str + i, string.len - i);
    if(!decode.error){
      i += decode.advance;
      result.str[result.len++] = decode.out_str;
    }
    else unicode_error(question_mark32);
  }
  result.str[result.len] = 0;
  return result;
}

function String16
string8_to_string16(Allocator *allocator, String in){
  String16 result = {exp_alloc_array(allocator, U16, (in.len*2)+1)}; // @Note(Krzosa): Should be more then enough space
  for(S64 i = 0; i < in.len;){
    UTF32_Result decode = utf8_to_utf32(in.str + i, in.len - i);
    if(!decode.error){
      i += decode.advance;
      UTF16_Result encode = utf32_to_utf16(decode.out_str);
      if(!encode.error){
        for(S32 j = 0; j < encode.len; j++){
          result.str[result.len++] = encode.out_str[j];
        }
      }
      else unicode_error(question_mark16);
    }
    else unicode_error(question_mark16);
  }

  result.str[result.len] = 0;
  return result;
}

function String
string16_to_string8(Allocator *allocator, String16 in){
  String result = {exp_alloc_array(allocator, U8, in.len*4+1)};
  for(S64 i = 0; i < in.len;){
    UTF32_Result decode = utf16_to_utf32(in.str + i, in.len - i);
    if(!decode.error){
      i += decode.advance;
      UTF8_Result encode = utf32_to_utf8(decode.out_str);
      if(!encode.error){
        for(S32 j = 0; j < encode.len; j++)
          result.str[result.len++] = encode.out_str[j];
      }
      else unicode_error(question_mark8);
    }
    else unicode_error(question_mark8);
  }

  result.str[result.len] = 0;
  return result;
}

function B32
string_compare(String16 a, String16 b){
  if(a.len != b.len) return false;
  for(S64 i = 0; i < a.len; i++){
    if(a.str[i] != b.str[i]) return false;
  }
  return true;
}

function B32
string_compare(String32 a, String32 b){
  if(a.len != b.len) return false;
  for(S64 i = 0; i < a.len; i++){
    if(a.str[i] != b.str[i]) return false;
  }
  return true;
}

function S64
widechar_len(wchar_t *string){
  S64 len = 0;
  while(*string++!=0)len++;
  return len;
}

function String16
string16_from_widechar(wchar_t *string){
  String16 result;
  result.str = (U16 *)string;
  result.len = widechar_len(string);
  return result;
}

function String
string16_copy(Allocator *a, String string){
  U8 *copy = exp_alloc_array(a, U8, string.len+1);
  memory_copy(copy, string.str, string.len);
  copy[string.len] = 0;
  return String{copy, string.len};
}

function void
test_unicode(){
  assert(utf8_to_utf32((U8 *)"A", 1).out_str == 'A');
  assert(utf8_to_utf32((U8 *)"ć", 2).out_str == 0x107);
  assert(utf8_to_utf32((U8 *)"ó", 2).out_str == 0x000000f3);

  Scratch scratch;
  String32 result = string8_to_string32(scratch, "Thing"_s);
  assert(result.len == 5);
  assert(result.str[0] == 'T');
  assert(result.str[4] == 'g');
  result = string8_to_string32(scratch, "óźćłŁQ42-=?"_s);
  {
    U16 u16_code[] = {0x0054, 0x0068, 0x0069, 0x006e, 0x0067, 0x0020, 0x0069, 0x0073};
    String16 thing_is16 = {u16_code, buff_cap(u16_code)};
    String string = "Thing is"_s;
    String16 thing_is_convert = string8_to_string16(scratch, string);
    assert(string_compare(thing_is16, thing_is_convert));

    String32 a = string16_to_string32(scratch, thing_is16);
    String32 b = string16_to_string32(scratch, thing_is_convert);
    String32 c = string8_to_string32 (scratch, string);
    assert(string_compare(a, b) && string_compare(b,c));
  }

  {
    U16 code[] = {0x015a, 0x0104, 0x00f3, 0x015b, 0x017a, 0x0107, 0x0144, 0x0143, 0x0119, 0x00f3};
    String16 code_str = {code, buff_cap(code)};
    String string = "ŚĄóśźćńŃęó"_s;
    String16 convert = string8_to_string16(scratch, string);
    assert(string_compare(convert, code_str));

    String32 a = string16_to_string32(scratch, code_str);
    String32 b = string16_to_string32(scratch, convert);
    String32 c = string8_to_string32 (scratch, string);
    assert(string_compare(a, b) && string_compare(b,c));
  }

  {
    String str = "    ം ഃ അ ആ ഇ ഈ ഉ ഊ ഋ ഌ എ ഏ ഐ ഒ ഓ ഔ ക ഖ ഗ ഘ ങ ച ഛ ജ ഝ ഞ ട ഠ ഡ ഢ ണ ത ഥ ദ ധ ന പ ഫ ബ ഭ മ യ ര റ ല ള ഴ വ ശ ഷ സ ഹ ാ ി ീ ു ൂ ൃ െ േ ൈ ൊ ോ ൌ ് ൗ ൠ ൡ ൦ ൧ ൨ ൩ ൪ ൫ ൬ ൭ ൮ ൯ "_s;
    U16 arr[] = {0x0020, 0x0020, 0x0020, 0x0020, 0x0d02, 0x0020, 0x0d03, 0x0020, 0x0d05, 0x0020, 0x0d06, 0x0020, 0x0d07, 0x0020, 0x0d08, 0x0020, 0x0d09, 0x0020, 0x0d0a, 0x0020, 0x0d0b, 0x0020, 0x0d0c, 0x0020, 0x0d0e, 0x0020, 0x0d0f, 0x0020, 0x0d10, 0x0020, 0x0d12, 0x0020, 0x0d13, 0x0020, 0x0d14, 0x0020, 0x0d15, 0x0020, 0x0d16, 0x0020, 0x0d17, 0x0020, 0x0d18, 0x0020, 0x0d19, 0x0020, 0x0d1a, 0x0020, 0x0d1b, 0x0020, 0x0d1c, 0x0020, 0x0d1d, 0x0020, 0x0d1e, 0x0020, 0x0d1f, 0x0020, 0x0d20, 0x0020, 0x0d21, 0x0020, 0x0d22, 0x0020, 0x0d23, 0x0020, 0x0d24, 0x0020, 0x0d25, 0x0020, 0x0d26, 0x0020, 0x0d27, 0x0020, 0x0d28, 0x0020, 0x0d2a, 0x0020, 0x0d2b, 0x0020, 0x0d2c, 0x0020, 0x0d2d, 0x0020, 0x0d2e, 0x0020, 0x0d2f, 0x0020, 0x0d30, 0x0020, 0x0d31, 0x0020, 0x0d32, 0x0020, 0x0d33, 0x0020, 0x0d34, 0x0020, 0x0d35, 0x0020, 0x0d36, 0x0020, 0x0d37, 0x0020, 0x0d38, 0x0020, 0x0d39, 0x0020, 0x0d3e, 0x0020, 0x0d3f, 0x0020, 0x0d40, 0x0020, 0x0d41, 0x0020, 0x0d42, 0x0020, 0x0d43, 0x0020, 0x0d46, 0x0020, 0x0d47, 0x0020, 0x0d48, 0x0020, 0x0d4a, 0x0020, 0x0d4b, 0x0020, 0x0d4c, 0x0020, 0x0d4d, 0x0020, 0x0d57, 0x0020, 0x0d60, 0x0020, 0x0d61, 0x0020, 0x0d66, 0x0020, 0x0d67, 0x0020, 0x0d68, 0x0020, 0x0d69, 0x0020, 0x0d6a, 0x0020, 0x0d6b, 0x0020, 0x0d6c, 0x0020, 0x0d6d, 0x0020, 0x0d6e, 0x0020, 0x0d6f, 0x0020};
    String16 a = {arr, buff_cap(arr)};
    String16 b = string8_to_string16(scratch, str);
    assert(string_compare(a,b));

    String32 c = string16_to_string32(scratch, a);
    String32 d = string16_to_string32(scratch, b);
    assert(string_compare(c,d));

    String e = string16_to_string8(scratch, a);
    String f = string16_to_string8(scratch, b);
    assert(string_compare(e,f));
  }

  {
    String str = "    豈 更 車 賈 滑 串 句 龜 龜 契 金 喇 奈 懶 癩 羅 蘿 螺 裸 邏 樂 洛 烙 珞 落 酪 駱 亂 卵 欄 爛 蘭 鸞 嵐 濫 藍 襤 拉 臘 蠟 廊 朗 浪 狼 郎 來 冷 勞 擄 櫓 爐 盧 老 蘆 虜 路 露 魯 鷺 碌 祿 綠 菉 錄 鹿 論 壟 弄 籠 聾 牢 磊 賂 雷 壘 屢 樓 淚 漏 累 縷 陋 勒 肋 凜 凌 稜 綾 菱 陵 讀 拏 樂 諾 丹 寧 怒 率 異 北 磻 便 復 不 泌 數 索 參 塞 省 葉 說 殺 辰 沈 拾 若 掠 略 亮 兩 凉 梁 糧 良 諒 量 勵 ..."_s;
    U16 arr[] = {0x0020, 0x0020, 0x0020, 0x0020, 0xf900, 0x0020, 0xf901, 0x0020, 0xf902, 0x0020, 0xf903, 0x0020, 0xf904, 0x0020, 0xf905, 0x0020, 0xf906, 0x0020, 0xf907, 0x0020, 0xf908, 0x0020, 0xf909, 0x0020, 0xf90a, 0x0020, 0xf90b, 0x0020, 0xf90c, 0x0020, 0xf90d, 0x0020, 0xf90e, 0x0020, 0xf90f, 0x0020, 0xf910, 0x0020, 0xf911, 0x0020, 0xf912, 0x0020, 0xf913, 0x0020, 0xf914, 0x0020, 0xf915, 0x0020, 0xf916, 0x0020, 0xf917, 0x0020, 0xf918, 0x0020, 0xf919, 0x0020, 0xf91a, 0x0020, 0xf91b, 0x0020, 0xf91c, 0x0020, 0xf91d, 0x0020, 0xf91e, 0x0020, 0xf91f, 0x0020, 0xf920, 0x0020, 0xf921, 0x0020, 0xf922, 0x0020, 0xf923, 0x0020, 0xf924, 0x0020, 0xf925, 0x0020, 0xf926, 0x0020, 0xf927, 0x0020, 0xf928, 0x0020, 0xf929, 0x0020, 0xf92a, 0x0020, 0xf92b, 0x0020, 0xf92c, 0x0020, 0xf92d, 0x0020, 0xf92e, 0x0020, 0xf92f, 0x0020, 0xf930, 0x0020, 0xf931, 0x0020, 0xf932, 0x0020, 0xf933, 0x0020, 0xf934, 0x0020, 0xf935, 0x0020, 0xf936, 0x0020, 0xf937, 0x0020, 0xf938, 0x0020, 0xf939, 0x0020, 0xf93a, 0x0020, 0xf93b, 0x0020, 0xf93c, 0x0020, 0xf93d, 0x0020, 0xf93e, 0x0020, 0xf93f, 0x0020, 0xf940, 0x0020, 0xf941, 0x0020, 0xf942, 0x0020, 0xf943, 0x0020, 0xf944, 0x0020, 0xf945, 0x0020, 0xf946, 0x0020, 0xf947, 0x0020, 0xf948, 0x0020, 0xf949, 0x0020, 0xf94a, 0x0020, 0xf94b, 0x0020, 0xf94c, 0x0020, 0xf94d, 0x0020, 0xf94e, 0x0020, 0xf94f, 0x0020, 0xf950, 0x0020, 0xf951, 0x0020, 0xf952, 0x0020, 0xf953, 0x0020, 0xf954, 0x0020, 0xf955, 0x0020, 0xf956, 0x0020, 0xf957, 0x0020, 0xf958, 0x0020, 0xf959, 0x0020, 0xf95a, 0x0020, 0xf95b, 0x0020, 0xf95c, 0x0020, 0xf95d, 0x0020, 0xf95e, 0x0020, 0xf95f, 0x0020, 0xf960, 0x0020, 0xf961, 0x0020, 0xf962, 0x0020, 0xf963, 0x0020, 0xf964, 0x0020, 0xf965, 0x0020, 0xf966, 0x0020, 0xf967, 0x0020, 0xf968, 0x0020, 0xf969, 0x0020, 0xf96a, 0x0020, 0xf96b, 0x0020, 0xf96c, 0x0020, 0xf96d, 0x0020, 0xf96e, 0x0020, 0xf96f, 0x0020, 0xf970, 0x0020, 0xf971, 0x0020, 0xf972, 0x0020, 0xf973, 0x0020, 0xf974, 0x0020, 0xf975, 0x0020, 0xf976, 0x0020, 0xf977, 0x0020, 0xf978, 0x0020, 0xf979, 0x0020, 0xf97a, 0x0020, 0xf97b, 0x0020, 0xf97c, 0x0020, 0xf97d, 0x0020, 0xf97e, 0x0020, 0xf97f, 0x0020, 0x002e, 0x002e, 0x002e};
    String16 a = {arr, buff_cap(arr)};
    String16 b = string8_to_string16(scratch, str);
    assert(string_compare(a,b));

    String32 c = string16_to_string32(scratch, a);
    String32 d = string16_to_string32(scratch, b);
    assert(string_compare(c,d));

    String e = string16_to_string8(scratch, a);
    String f = string16_to_string8(scratch, b);
    assert(string_compare(e,f));
  }

  {
    String str = "    ！ ＂ ＃ ＄ ％ ＆ ＇ （ ） ＊ ＋ ， － ． ／ ０ １ ２ ３ ４ ５ ６ ７ ８ ９ ： ； ＜ ＝ ＞ ？ ＠ Ａ Ｂ Ｃ Ｄ Ｅ Ｆ Ｇ Ｈ Ｉ Ｊ Ｋ Ｌ Ｍ Ｎ Ｏ Ｐ Ｑ Ｒ Ｓ Ｔ Ｕ Ｖ Ｗ Ｘ Ｙ Ｚ ［ ＼ ］ ＾ ＿ ｀ ａ ｂ ｃ ｄ ｅ ｆ ｇ ｈ ｉ ｊ ｋ ｌ ｍ ｎ ｏ ｐ ｑ ｒ ｓ ｔ ｕ ｖ ｗ ｘ ｙ ｚ ｛ ｜ ｝ ～ ｡ ｢ ｣ ､ ･ ｦ ｧ ｨ ｩ ｪ ｫ ｬ ｭ ｮ ｯ ｰ ｱ ｲ ｳ ｴ ｵ ｶ ｷ ｸ ｹ ｺ ｻ ｼ ｽ ｾ ｿ ﾀ ﾁ ﾂ ... "_s;
    U16 arr[] = {0x0020, 0x0020, 0x0020, 0x0020, 0xff01, 0x0020, 0xff02, 0x0020, 0xff03, 0x0020, 0xff04, 0x0020, 0xff05, 0x0020, 0xff06, 0x0020, 0xff07, 0x0020, 0xff08, 0x0020, 0xff09, 0x0020, 0xff0a, 0x0020, 0xff0b, 0x0020, 0xff0c, 0x0020, 0xff0d, 0x0020, 0xff0e, 0x0020, 0xff0f, 0x0020, 0xff10, 0x0020, 0xff11, 0x0020, 0xff12, 0x0020, 0xff13, 0x0020, 0xff14, 0x0020, 0xff15, 0x0020, 0xff16, 0x0020, 0xff17, 0x0020, 0xff18, 0x0020, 0xff19, 0x0020, 0xff1a, 0x0020, 0xff1b, 0x0020, 0xff1c, 0x0020, 0xff1d, 0x0020, 0xff1e, 0x0020, 0xff1f, 0x0020, 0xff20, 0x0020, 0xff21, 0x0020, 0xff22, 0x0020, 0xff23, 0x0020, 0xff24, 0x0020, 0xff25, 0x0020, 0xff26, 0x0020, 0xff27, 0x0020, 0xff28, 0x0020, 0xff29, 0x0020, 0xff2a, 0x0020, 0xff2b, 0x0020, 0xff2c, 0x0020, 0xff2d, 0x0020, 0xff2e, 0x0020, 0xff2f, 0x0020, 0xff30, 0x0020, 0xff31, 0x0020, 0xff32, 0x0020, 0xff33, 0x0020, 0xff34, 0x0020, 0xff35, 0x0020, 0xff36, 0x0020, 0xff37, 0x0020, 0xff38, 0x0020, 0xff39, 0x0020, 0xff3a, 0x0020, 0xff3b, 0x0020, 0xff3c, 0x0020, 0xff3d, 0x0020, 0xff3e, 0x0020, 0xff3f, 0x0020, 0xff40, 0x0020, 0xff41, 0x0020, 0xff42, 0x0020, 0xff43, 0x0020, 0xff44, 0x0020, 0xff45, 0x0020, 0xff46, 0x0020, 0xff47, 0x0020, 0xff48, 0x0020, 0xff49, 0x0020, 0xff4a, 0x0020, 0xff4b, 0x0020, 0xff4c, 0x0020, 0xff4d, 0x0020, 0xff4e, 0x0020, 0xff4f, 0x0020, 0xff50, 0x0020, 0xff51, 0x0020, 0xff52, 0x0020, 0xff53, 0x0020, 0xff54, 0x0020, 0xff55, 0x0020, 0xff56, 0x0020, 0xff57, 0x0020, 0xff58, 0x0020, 0xff59, 0x0020, 0xff5a, 0x0020, 0xff5b, 0x0020, 0xff5c, 0x0020, 0xff5d, 0x0020, 0xff5e, 0x0020, 0xff61, 0x0020, 0xff62, 0x0020, 0xff63, 0x0020, 0xff64, 0x0020, 0xff65, 0x0020, 0xff66, 0x0020, 0xff67, 0x0020, 0xff68, 0x0020, 0xff69, 0x0020, 0xff6a, 0x0020, 0xff6b, 0x0020, 0xff6c, 0x0020, 0xff6d, 0x0020, 0xff6e, 0x0020, 0xff6f, 0x0020, 0xff70, 0x0020, 0xff71, 0x0020, 0xff72, 0x0020, 0xff73, 0x0020, 0xff74, 0x0020, 0xff75, 0x0020, 0xff76, 0x0020, 0xff77, 0x0020, 0xff78, 0x0020, 0xff79, 0x0020, 0xff7a, 0x0020, 0xff7b, 0x0020, 0xff7c, 0x0020, 0xff7d, 0x0020, 0xff7e, 0x0020, 0xff7f, 0x0020, 0xff80, 0x0020, 0xff81, 0x0020, 0xff82, 0x0020, 0x002e, 0x002e, 0x002e, 0x0020};
    String16 a = {arr, buff_cap(arr)};
    String16 b = string8_to_string16(scratch, str);
    assert(string_compare(a,b));

    String32 c = string16_to_string32(scratch, a);
    String32 d = string16_to_string32(scratch, b);
    assert(string_compare(c,d));

    String e = string16_to_string8(scratch, a);
    String f = string16_to_string8(scratch, b);
    assert(string_compare(e,f));
  }

  { // With surrogate pairs
    U8 arr8[] = {0xf0, 0x90, 0x90, 0x90, 0xf0, 0x90, 0x90, 0x91};
    String str = {arr8, buff_cap(arr8)};
    U16 arr[] = {0xd801, 0xdc10, 0xd801, 0xdc11};
    String16 a = {arr, buff_cap(arr)};
    String16 b = string8_to_string16(scratch, str);
    assert(string_compare(a,b));

    String32 c = string16_to_string32(scratch, a);
    String32 d = string16_to_string32(scratch, b);
    assert(string_compare(c,d));

    String e = string16_to_string8(scratch, a);
    String f = string16_to_string8(scratch, b);
    assert(string_compare(e,f));
  }

}
