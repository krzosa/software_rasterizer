enum ProfileScopeName {
  ProfileScopeName_draw_triangle,
  ProfileScopeName_Count,
};

struct ProfileScope {
  U64 samples[5096];
  S64 i;
};

global ProfileScope profile_scopes[ProfileScopeName_Count];

#define PROFILE_BEGIN(name) do { \
ProfileScope *__profile_scope = profile_scopes + ProfileScopeName_##name; \
__profile_scope->samples[__profile_scope->i] = __rdtsc(); \
} while (0)

#define PROFILE_END(name) do { \
ProfileScope *_profile_scope = profile_scopes + ProfileScopeName_##name; \
_profile_scope->samples[_profile_scope->i] = __rdtsc() - _profile_scope->samples[_profile_scope->i]; \
_profile_scope->i = (_profile_scope->i + 1) % 5096; \
}while (0)


function void save_profile_data(ProfileScope *scope, S8 scenario_name, S8 scope_name) {
  /*for (S64 si = 1; si < scope->i; si++) {
    for (S64 sj = 1; sj < scope->i; sj++) {
      if (scope->samples[sj] < scope->samples[sj - 1]) {
        F64 temp = scope->samples[sj];
        scope->samples[sj] = scope->samples[sj-1];
        scope->samples[sj-1] = temp;
      }
    }
  }*/
  
  /*   
    Scratch scratch;
    scenario_name = string_chop_last_period(scenario_name);
    scenario_name = string_skip_to_last_slash(scenario_name);
    U8 *string_pointer = string_begin(scratch);
    string_fmt(scratch, "%s %s\n", build_name, scenario_name);
    S64 one_past_last = scope->i;
    for (S64 si = 0; si < one_past_last; si++) {
      string_fmt(scratch, "%u\n", scope->samples[si]);
    }
    
    S8 data = string_end(scratch, string_pointer);
    Date date = os_date();
    os_make_dir(LIT("stats"));
    S8 name = string_fmt(scratch, "stats/%s_%s_%s_%u_%u_%u_%u_%u_%u.txt", scope_name, build_name, scenario_name, date.year, date.month, date.day, date.hour, date.minute, date.second);
    os_append_file(name, data);
   */
}