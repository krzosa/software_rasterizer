#include "multimedia.cpp"
#include "obj.cpp"

#define STBI_ASSERT assert
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

enum class Obj_Token_Type {
  none, word, number, whitespace, end
};

struct Obj_Token {
  Obj_Token_Type type;
  double number;
  union {
    struct {
      char* s;
      int len;
    };
    String s8;
  };
};

function Bitmap 
load_image(String path) {
  Scratch scratch;
  String file = os_read_file(scratch, path);
  
  int x, y, n;
  unsigned char* data = stbi_load_from_memory(file.str, file.len, &x, &y, &n, 4);
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

function Obj_Token next_token_raw(char** data) {
  Obj_Token result = {};
  result.s = *data;
  *data += 1;
  
  if (is_alphabetic(*result.s)) {
    result.type = Obj_Token_Type::word;
    while (!is_whitespace(**data)) {
      *data += 1;
    }
    result.len = (int)(*data - result.s);
  }
  else if (is_number(*result.s) || *result.s == '-') {
    result.type = Obj_Token_Type::number;
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
    result.type = Obj_Token_Type::whitespace;
    while (is_whitespace(**data)) *data += 1;
    result.len = (int)(*data - result.s);
  }
  else if (*result.s == 0) {
    result.type = Obj_Token_Type::end;
  }
  else if (*result.s >= '!') {
    result.type = (Obj_Token_Type)*result.s;
  }
  
  return result;
}

function Obj_Token next_token(char** data) {
  Obj_Token result;
  do {
    result = next_token_raw(data);
  } while (result.type == Obj_Token_Type::whitespace);
  return result;
}

function double expect_number(char** data) {
  Obj_Token t = next_token(data);
  assert(t.type == Obj_Token_Type::number); // @Todo: Error handling, error flag
  return t.number;
}

function void expect_token(char** data, char token) {
  Obj_Token t = next_token(data);
  assert(t.type == (Obj_Token_Type)token); // @Todo: Error handling, error flag
}

function void debug_expect_raw(char** data, Obj_Token_Type type) {
  char* data_temp = *data;
  Obj_Token t = next_token_raw(&data_temp);
  assert(t.type == type);
}

function void 
parse_mtl(Obj* obj, String path_obj_folder, String mtl_file) {
  Scratch scratch;
  char *data = (char *)mtl_file.str;
  Obj_Material *m = 0;
  
  for (;;) {
    Obj_Token token = next_token(&data);
    if (token.type == Obj_Token_Type::end) break;
    else if (token.type == Obj_Token_Type::word) {
      if (string_compare(token.s8, "newmtl"_s)) {
        token = next_token(&data);
        m = obj->materials.push_empty_zero();
        m->name_len = clamp_top(token.len, 64);
        memory_copy(m->name, token.s8.str, m->name_len);
      }
      
      else if (string_compare(token.s8, "Ns"_s)) {
        m->shininess = expect_number(&data);
      }
      
      else if (string_compare(token.s8, "Ka"_s)) {
        m->ambient_color.x = expect_number(&data);
        m->ambient_color.y = expect_number(&data);
        m->ambient_color.z = expect_number(&data);
      }
      else if (string_compare(token.s8, "Kd"_s)) {
        m->diffuse_color.x = expect_number(&data);
        m->diffuse_color.y = expect_number(&data);
        m->diffuse_color.z = expect_number(&data);
      }
      
      else if (string_compare(token.s8, "Ks"_s)) {
        m->specular_color.x = expect_number(&data);
        m->specular_color.y = expect_number(&data);
        m->specular_color.z = expect_number(&data);
      }
      else if (string_compare(token.s8, "Ni"_s)) {
        m->optical_density = expect_number(&data);
      }
      
      else if (string_compare(token.s8, "d"_s)) {
        m->non_transparency = expect_number(&data);
      }
      else if (string_compare(token.s8, "illum"_s)) {
        m->illumination_model = (S32)expect_number(&data);
      }
      
      else if (string_compare(token.s8, "map_Kd"_s)) {
        Obj_Token t = next_token(&data);
        String path = string_fmt(scratch, "%Q/%Q\0", path_obj_folder, t.s8);
        m->texture_diffuse = load_image(path);
      }
      
      else if (string_compare(token.s8, "map_Ka"_s)) {
        Obj_Token t = next_token(&data);
        String path = string_fmt(scratch, "%Q/%Q\0", path_obj_folder, t.s8);
        m->texture_ambient = load_image(path);
      }
      
      else if (string_compare(token.s8, "map_d"_s)) {
        Obj_Token t = next_token(&data);
        String path = string_fmt(scratch, "%Q/%Q\0", path_obj_folder, t.s8);
        m->texture_dissolve = load_image(path);
      }
      
      else if (string_compare(token.s8, "map_Disp"_s)) {
        Obj_Token t = next_token(&data);
        String path = string_fmt(scratch, "%Q/%Q\0", path_obj_folder, t.s8);
        m->texture_displacement = load_image(path);
      }
    }
  }
}

function Obj 
parse(Allocator *allocator, char* data, String path_obj_folder) {
  Set_Allocator(allocator);
  Scratch mtl_scratch;
  Obj result = {};
  //result.vertices.init(allocator, 160000);
  //result.texture_coordinates.init(allocator, 160000);
  //result.normals.init(allocator, 160000);
  //result.mesh.init(allocator, 64);
  //result.materials.init(allocator, 64);
  int smoothing = 0;
  
  Obj_Mesh *mesh = result.mesh.push_empty_zero();
  //mesh->indices.init(allocator);
  int material_id = -1;
  
  S64 debug_i = 0;
  for (;;debug_i++){
    Obj_Token token = next_token(&data);
    if (token.type == Obj_Token_Type::end) break;
    else if (token.type == Obj_Token_Type::word) {
      if (string_compare(token.s8, "v"_s)) {
        Vec3 *vertex = result.vertices.push_empty_zero();
        vertex->x = (float)expect_number(&data);
        vertex->y = (float)expect_number(&data);
        vertex->z = (float)expect_number(&data);
        debug_expect_raw(&data, Obj_Token_Type::whitespace);
      }
      
      else if (string_compare(token.s8, "vt"_s)) {
        Vec2 *tex = result.texture_coordinates.push_empty_zero();
        tex->x = (float)expect_number(&data);
        tex->y = (float)expect_number(&data);
        debug_expect_raw(&data, Obj_Token_Type::whitespace);
      }
      
      else if (string_compare(token.s8, "vn"_s)) {
        Vec3 *norm = result.normals.push_empty_zero();
        norm->x = (float)expect_number(&data);
        norm->y = (float)expect_number(&data);
        norm->z = (float)expect_number(&data);
        debug_expect_raw(&data, Obj_Token_Type::whitespace);
      }
      
      else if (string_compare(token.s8, "mtllib"_s)) {
        Obj_Token t = next_token(&data);
        String path = string_fmt(mtl_scratch, "%Q/%Q", path_obj_folder, t.s8);
        String mtl_file = os_read_file(mtl_scratch, path);
        if(mtl_file.str) {
          parse_mtl(&result, path_obj_folder, mtl_file);  
        }
      }
      
      else if (string_compare(token.s8, "usemtl"_s)) {
        Obj_Token t = next_token(&data);
        assert(t.type == Obj_Token_Type::word);
        for(U64 i = 0; i < result.materials.len; i++) {
          Obj_Material *m = result.materials.data + i;
          if(string_compare({(U8 *)m->name, m->name_len}, t.s8)) {
            material_id = i;
            break;
          }
        }
      }
      
      else if (string_compare(token.s8, "o"_s)) {
        Obj_Token t = next_token(&data);
        assert(t.type == Obj_Token_Type::word);
        if (mesh->indices.len != 0) {
          mesh = result.mesh.push_empty_zero();
        }
        else {
          U64 len = clamp_top(t.len, 64);
          memory_copy(mesh->name, t.s, len);
        }
      }
      
      else if (string_compare(token.s8, "s"_s)) {
        Obj_Token t = next_token(&data);
        if (t.type == Obj_Token_Type::number) {
          smoothing = (int)t.number;
        }
        else {
          assert(t.type == Obj_Token_Type::word);
          if (string_compare(t.s8, "on"_s)) {
            smoothing = 1;
          }
          else if (string_compare(t.s8, "off"_s)) {
            smoothing = 0;
          }
          else invalid_codepath;
        }
        
      }
      
      else if (string_compare(token.s8, "g"_s)) {
        Obj_Token t = next_token(&data);
        assert(t.type == Obj_Token_Type::word);
      }
      
      else if (string_compare(token.s8, "f"_s)) {
        Obj_Index *i = mesh->indices.push_empty_zero();
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
        //debug_expect_raw(&data, Obj_Token_Type::whitespace);
      }
    }
  }
  return result;
}

function Obj 
load_obj(Allocator *arena, String file) {
  Scratch scratch;
  String data = os_read_file(scratch, file);
  assert(data.str);
  
  String path = string_chop_last_slash(file);
  Obj result = parse(arena, (char *)data.str, path);
  result.name = file;
  return result;
}

template<class T> void 
dump_array(String_Builder *sb, Array<T> *arr){
  sb->append_data(arr, sizeof(*arr));
  sb->append_data(arr->data, sizeof(T)*arr->len);
}

function void
dump_bitmap_image(String_Builder *sb, Bitmap *bm){
  sb->append_data(bm->pixels, sizeof(U32)*bm->x*bm->y);
}

function B32
_os_write_file(String file, String data, B32 append = false) {
  B32 result = false;
  DWORD access = GENERIC_WRITE;
  DWORD creation_disposition = CREATE_ALWAYS;
  if (append) {
    access = FILE_APPEND_DATA;
    creation_disposition = OPEN_ALWAYS;
  }
  
  // @Todo(Krzosa): Unicode
  HANDLE handle = CreateFileA((const char *)file.str, access, 0, NULL, creation_disposition, FILE_ATTRIBUTE_NORMAL, NULL);
  if (handle != INVALID_HANDLE_VALUE) {
    DWORD bytes_written = 0;
    // @Todo: can only read 32 byte size files?
    assert_msg(data.len == (U32)data.len, "Max data size os_write can handle is 32 bytes, data to write is larger then 32 bytes!");
    B32 error = WriteFile(handle, data.str, (U32)data.len, &bytes_written, NULL);
    if (error == false) log_error("Failed to write to file: %Q", file);
    else {
      if (bytes_written != data.len) log_error("Failed to write to file: %Q, mismatch between length requested to write and length written", file);
      else result = true;
    }
    CloseHandle(handle);
  } 
  else {
    log_error("File not found when trying to write: %Q", file);
  }
  
  return result;
}

function B32 
os_write_file(String file, String data) {
  return _os_write_file(file, data, false);
}
function B32 
os_append_file(String file, String data) {
  return _os_write_file(file, data, true);
}

function void
dump_obj_to_file(Obj *obj, String out_name){
  obj->vertices.allocator = 0;
  obj->vertices.cap = obj->vertices.len;
  
  obj->texture_coordinates.allocator = 0;
  obj->texture_coordinates.cap = obj->texture_coordinates.len;
  
  obj->normals.allocator = 0;
  obj->normals.cap = obj->normals.len;
  
  obj->mesh.allocator = 0;
  obj->mesh.cap = obj->mesh.len;
  
  obj->materials.allocator = 0;
  obj->materials.cap = obj->materials.len;
  
  Iter(obj->mesh){
    it->indices.allocator = 0;
    it->indices.cap = it->indices.len;
  }
  
  Scratch arena;
  String_Builder sb = string_builder_make(arena, mib(4));
  sb.append_data(obj, sizeof(Obj));
  sb.append_data(obj->name.str, obj->name.len);
  sb.append_data(obj->vertices.data, obj->vertices.len*sizeof(Vec3));
  sb.append_data(obj->texture_coordinates.data, obj->texture_coordinates.len*sizeof(Vec2));
  sb.append_data(obj->normals.data, obj->normals.len*sizeof(Vec3));
  sb.append_data(obj->mesh.data, obj->mesh.len*sizeof(Obj_Mesh));
  sb.append_data(obj->materials.data, obj->materials.len*sizeof(Obj_Material));
  
  Iter(obj->mesh){
    sb.append_data(it->indices.data, sizeof(Obj_Index)*it->indices.len);
  }
  
  Iter(obj->materials){
    sb.append_data(it, sizeof(Obj_Material));
    dump_bitmap_image(&sb, &it->texture_ambient);
    dump_bitmap_image(&sb, &it->texture_diffuse);
    dump_bitmap_image(&sb, &it->texture_dissolve);
    dump_bitmap_image(&sb, &it->texture_displacement);
  }
  
  String result = string_flatten(arena, &sb);
  os_write_file(out_name, result);
}

global FILE *output_file;

function void
asset_log(Log_Kind kind, String string, char *file, int line){
  if(!output_file) {
    
  }
  fprintf(output_file, "%.*s", string_expand(string));
}

int 
main(int argc, char **argv){
  output_file = fopen("asset.log.txt", "a");
  thread_ctx.log_proc = asset_log;
  
  Obj sponza_obj = load_obj(&os_process_heap, "assets/sponza/sponza.obj"_s);
  dump_obj_to_file(&sponza_obj, "sponza.bin"_s);
  
  Obj plane_obj = load_obj(&pernament_arena, "assets/f22.obj"_s);
  
  // Add brick texture as main texture
  Scratch scratch;
  Set_Allocator(scratch);
  Obj_Material material = {};
  material.texture_ambient = load_image("assets/bricksx64.png"_s);
  plane_obj.materials.add(material);
  For(plane_obj.mesh){
    IFor(it->indices, jt, j){
      jt->material_id = 0;
    }
  }
  
  dump_obj_to_file(&plane_obj, "plane.bin"_s);
  fclose(output_file);
}