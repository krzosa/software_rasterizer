///
/// [?] - Cache bitmaps
/// [ ] - Fix potential portability issues due to compiler struct alignment differences etc.
///

struct Obj_Index {
  int vertex[3];
  int tex[3];
  int normal[3];
  S32 material_id;
  S32 smoothing_group_id;
};

struct Obj_Mesh {
  char name[64];
  Array<Obj_Index> indices;
};

struct Obj_Material {
  char name[64];
  U32  name_len;
  Bitmap texture_ambient; // map_Ka
  Bitmap texture_diffuse; // map_Kd
  Bitmap texture_dissolve; // map_d
  Bitmap texture_displacement; // map_Disp
  F32 non_transparency; // d
  F32 transparency; // Tr
  F32 optical_density; // Ni
  F32 shininess; // Ns
  S32 illumination_model; // illum 
  Vec3 ambient_color; // Ka
  Vec3 diffuse_color; // Kd
  Vec3 specular_color; // Ks
};

struct Obj {
  String name;
  Array<Vec3> vertices;
  Array<Vec2> texture_coordinates;
  Array<Vec3> normals;
  Array<Obj_Mesh> mesh;
  Array<Obj_Material> materials;
};

struct Stream{
  U8 *cursor;
  U8 *end;
};

#define stream_read_array(s,T,c) (T *)stream_read(s,sizeof(T)*(c))
#define stream_read_struct(s,T) stream_read_array(s,T,1)
function void *
stream_read(Stream *s, SizeU size){
  U8 *result = s->cursor;
  s->cursor += size;
  assert(s->end >= s->cursor);
  return result;
}

function Obj *
load_obj_dump(Allocator *allocator, String filename){
  String string = os_read_file(allocator, filename);
  
  Obj *obj = (Obj *)string.str;
  Stream stream = {(U8 *)(obj+1), string.str + string.len};
  obj->name.str = stream_read_array(&stream, U8, obj->name.len);
  obj->vertices.data = stream_read_array(&stream, Vec3, obj->vertices.len);
  obj->texture_coordinates.data = stream_read_array(&stream, Vec2, obj->texture_coordinates.len);
  obj->normals.data = stream_read_array(&stream, Vec3, obj->normals.len);
  obj->mesh.data = stream_read_array(&stream, Obj_Mesh, obj->mesh.len);
  obj->materials.data = stream_read_array(&stream, Obj_Material, obj->materials.len);
  
  Iter(obj->mesh){
    it->indices.data = stream_read_array(&stream, Obj_Index, it->indices.len);
  }
  
  Iter(obj->materials){
    it->texture_ambient.pixels = stream_read_array(&stream, U32, it->texture_ambient.x*it->texture_ambient.y);
    it->texture_diffuse.pixels = stream_read_array(&stream, U32, it->texture_diffuse.x*it->texture_diffuse.y);
    it->texture_dissolve.pixels = stream_read_array(&stream, U32, it->texture_dissolve.x*it->texture_dissolve.y);
    it->texture_displacement.pixels = stream_read_array(&stream, U32, it->texture_displacement.x*it->texture_displacement.y);
  }
  
  return obj;
}

