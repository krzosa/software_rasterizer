#include "main.h"

struct Image {
  U32* pixels;
  I64 x;
  I64 y;
};

struct OSInitArgs {
  int window_x; 
  int window_y;
};

extern Image screen;
extern bool keydown_a;
extern bool keydown_b;
extern bool keydown_f1;
extern bool keydown_f2;
extern bool keydown_f3;

bool OS_GameLoop();
void OS_Init(OSInitArgs);
char* OS_ReadFile(const char* path);