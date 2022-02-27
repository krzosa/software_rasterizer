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


FN void save_profile_data(ProfileScope *scope, S8 scenario_name, S8 scope_name) {
  /*for (I64 si = 1; si < scope->i; si++) {
    for (I64 sj = 1; sj < scope->i; sj++) {
      if (scope->samples[sj] < scope->samples[sj - 1]) {
        F64 temp = scope->samples[sj];
        scope->samples[sj] = scope->samples[sj-1];
        scope->samples[sj-1] = temp;
      }
    }
  }*/

  Scratch scratch;
  scenario_name = string_chop_last_period(scenario_name);
  scenario_name = string_skip_to_last_slash(scenario_name);
  U8 *string_pointer = string_begin(scratch);
  S8 build_name = BUILD_NAME;
  string_format(scratch, "%s %s\n", build_name, scenario_name);
  I64 one_past_last = scope->i;
  for (I64 si = 0; si < one_past_last; si++) {
    string_format(scratch, "%f\n", scope->samples[si]);
  }

  S8 data = string_end(scratch, string_pointer);
  Date date = os_date();
  os_make_dir(LIT("stats"));
  S8 name = string_format(scratch, "stats/%s_%s_%s_%u_%u_%u_%u_%u_%u.txt", scope_name, build_name, scenario_name, date.year, date.month, date.day, date.hour, date.minute, date.second);
  os_append_file(name, data);
}