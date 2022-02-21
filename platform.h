// #include "main.h"
struct Image {
  U32* pixels;
  I64 x;
  I64 y;
};
#if 0

struct OSInitArgs {
  int window_x; 
  int window_y;
};

struct OS {
  Image screen;
  bool keydown_a;
  bool keydown_b;
  bool keydown_f1;
  bool keydown_f2;
  bool keydown_f3;
  bool app_is_running;

  void  init(OSInitArgs args);
  bool  game_loop();

  void  message(int debug, const char *msg, ...);
  char *read_file(const char *path);

  OS() = default;
  OS(OSInitArgs args) { init(args);  }
  char os_internal_data[1024];
};
#endif