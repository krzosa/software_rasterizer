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


FN void save_profile_data(ProfileScope *scope, S8 scenario_name) {
  for (I64 si = 1; si < scope->i; si++) {
    for (I64 sj = 1; sj < scope->i; sj++) {
      if (scope->samples[sj] < scope->samples[sj - 1]) {
        F64 temp = scope->samples[sj];
        scope->samples[sj] = scope->samples[sj-1];
        scope->samples[sj-1] = temp;
      }
    }
  }

  {
    Scratch scratch;
    U8 *string_pointer = string_begin(scratch);

    I64 one_past_last = scope->i;
    F64 sum = 0;
    for (I64 si = 0; si < one_past_last; si++) {
      sum += scope->samples[si];
      //string_format(scratch, "%f;", scope->samples[si]);
    }
    I64 index25perc = one_past_last / 4 - 1;
    F64 min = scope->samples[0];
    F64 percentile25 = scope->samples[index25perc];
    F64 median = scope->samples[one_past_last / 2 - 1];
    F64 percentile75 = scope->samples[index25perc*3];
    F64 max = scope->samples[one_past_last - 1];
    F64 avg = sum / scope->i;

    S8 build_name = BUILD_NAME;
    string_format(scratch, "%s_%s = min:%f 25%%:%f median:%f 75%%:%f max: %f avg:%f\n", build_name, scenario_name, min, percentile25, median, percentile75, max, avg);
    S8 data = string_end(scratch, string_pointer);
    os_append_file(LIT("data.txt"), data);
  }
}