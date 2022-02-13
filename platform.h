#include "main.h"
#include <stdint.h>
struct Image {
  uint32_t* pixels;
  int x;
  int y;
};

struct OSInitArgs {
  int window_x; 
  int window_y;
};

extern Image screen;
extern float* depth_buffer;
extern bool keydown_a;
extern bool keydown_b;

bool OS_GameLoop();
void OS_Init(OSInitArgs);