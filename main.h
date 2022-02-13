#pragma once
#define ASSERT(x) if(!(x)) *(volatile int *)0=0;
#define ARRAY_CAP(x) sizeof((x))/sizeof(*(x))
#define FUNCTION static
#define GLOBAL   static
#define MIN(x,y) (x)>(y)?(y):(x)
#define MAX(x,y) (x)>(y)?(x):(y)
