enum ProfileScopeName {
  ProfileScopeName_draw_triangle,
  ProfileScopeName_Count,
};

struct ProfileScope {
  F64 samples[5096];
  I64 i;
};

GLOBAL ProfileScope profile_scopes[ProfileScopeName_Count];

#define PROFILE_BEGIN(name) do { \
  ProfileScope *__profile_scope = profile_scopes + ProfileScopeName_##name; \
    __profile_scope->samples[__profile_scope->i] = os_time()*1000; \
} while (0)

#define PROFILE_END(name) do { \
  ProfileScope *_profile_scope = profile_scopes + ProfileScopeName_##name; \
    _profile_scope->samples[_profile_scope->i] = os_time()*1000 - _profile_scope->samples[_profile_scope->i]; \
      _profile_scope->i = (_profile_scope->i + 1) % 5096; \
}while (0)