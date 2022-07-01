enum ProfileScopeName {
  ProfileScopeName_draw_triangle,
  ProfileScopeName_fill_triangle,
  ProfileScopeName_draw_all_meshes,
  ProfileScopeName_draw_mesh,
  ProfileScopeName_draw_set_of_mesh_indices,
  ProfileScopeName_main_loop,
  ProfileScopeName_fill_triangle_after_depth_test,
  ProfileScopeName_Count,
};

const char *profile_scope_names[] = {
  "draw_triangle",
  "fill_triangle",
  "draw_all_meshes",
  "draw_mesh",
  "draw_set_of_mesh_indices",
  "main_loop",
  "fill_triangle_after_depth_test",
};

struct ProfileState {
  U64 samples[5096*32];
  S64 i;
};

global ProfileState profile_scopes[ProfileScopeName_Count];

force_inline void
profile_begin(ProfileScopeName name){
  ProfileState *p = profile_scopes + name;
  p->samples[p->i] = __rdtsc();
}

force_inline void
profile_end(ProfileScopeName name){
  ProfileState *p = profile_scopes + name;
  p->samples[p->i] = __rdtsc() - p->samples[p->i];
  p->i = (p->i + 1) % buff_cap(p->samples);
}

struct Profile_Scope{
  ProfileScopeName n;
  force_inline Profile_Scope(ProfileScopeName name){ profile_begin(name); n=name; }
  force_inline ~Profile_Scope(){ profile_end(n); }
};

#define PROFILE_BEGIN(name) profile_begin(ProfileScopeName_##name)
#define PROFILE_END(name) profile_end(ProfileScopeName_##name)
#define PROFILE_SCOPE(name) Profile_Scope profile_scope_##__LINE__(ProfileScopeName_##name)
