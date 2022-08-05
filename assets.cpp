#include "assets_obj_dump.cpp"

function void
asset_log(Log_Kind kind, String string, char *file, int line){
  printf("%.*s", string_expand(string));
}

int
main(int argc, char **argv){
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
    For_Named(it.indices, jt){
      jt.material_id = 0;
    }
  }

  dump_obj_to_file(&plane_obj, "plane.bin"_s);
}