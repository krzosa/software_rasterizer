#pragma once
struct Obj {
  float* vertices; // sets of 3 floats
  float* texture;  // sets of 2 floats
  float* normals;  // sets of 3 floats 
  int    vertices_count; // count of sets(vertices, textures, normals)

  int* indices; // layout: vvv/ttt/nnn
  int  indices_count;  // 1 count == 1 full face == 3vertex indices, 3 texture coordinate indices, 3 normal indices
  size_t memory_taken;
};

Obj  Obj_Parse(char* memory, size_t memory_size, char* data);
void Obj_Test();
