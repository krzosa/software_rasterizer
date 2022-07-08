#define NOMINMAX
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>

#include <float.h>
#include <stdint.h>
typedef int8_t   S8;
typedef int16_t  S16;
typedef int32_t  S32;
typedef int64_t  S64;
typedef uint8_t  U8;
typedef uint16_t U16;
typedef uint32_t U32;
typedef uint64_t U64;
typedef S8       B8;
typedef S16      B16;
typedef S32      B32;
typedef S64      B64;
typedef U64      SizeU;
typedef S64      SizeS;
typedef float    F32;
typedef double   F64;

#define U64MAX UINT64_MAX
#define U32MAX UINT32_MAX
#define U16MAX UINT16_MAX
#define U8MAX  UINT8_MAX
#define U64MIN 0
#define U32MIN 0
#define U16MIN 0
#define U8MIN  0
#define S64MAX INT64_MAX
#define S64MIN INT64_MIN
#define S32MAX INT32_MAX
#define S32MIN INT32_MIN
#define S16MAX INT16_MAX
#define S16MIN INT16_MIN
#define S8MAX  INT8_MAX
#define S8MIN  INT8_MIN
#define F32MAX FLT_MAX
#define F32MIN FLT_MIN
#define F64MAX DBL_MAX
#define F64MIN DBL_MIN

#define api
#define function
#define global
#define force_inline __forceinline
#define assert(x) do{if(!(x))__debugbreak();}while(0)
#define assert_msg(x,...) assert(x)
#define invalid_codepath assert_msg(0, "Invalid codepath")
#define invalid_return do{assert_msg(0, "Invalid codepath"); return {};}while(0)
#define invalid_default_case default: invalid_codepath
#define not_implemented assert_msg(0, "Not implemented")
#define unused(x) ((void)x)
#define buff_cap(x) (sizeof(x)/sizeof((x)[0]))
#define is_flag_set(val,flag) ((val) &  (flag))
#define set_flag(val,flag)    ((val) |= (flag))
#define unset_flag(val,flag)  ((val) &= (~(flag)))
#define bit_flag(x) (1ull << (x))
#define kib(x) ((x)*1024llu)
#define mib(x) (kib(x)*1024llu)
#define gib(x) (mib(x)*1024llu)
#define JOIN1(X,Y) X##Y  // helper macro
#define JOIN(X,Y) JOIN1(X,Y)
#define string_expand(x) (int)x.len, x.str

#define FLAG32(x) typedef U32 x; enum

#if defined(__clang__)
# define COMPILER_CLANG 1
# if defined(_WIN32)
#  define OS_WINDOWS 1
# elif defined(__linux__)
#  define OS_LINUX 1
# else
#  error Couldnt figure out the platform automatically
# endif
#elif defined(_MSC_VER)
# define COMPILER_MSVC 1
# define OS_WINDOWS 1
#elif defined(__GNUC__)
# define COMPILER_GCC 1
# if defined(__linux__)
#  define OS_LINUX 1
# endif
#else
# error Couldnt figure out the compiler
#endif

#if !defined(COMPILER_MSVC)
# define COMPILER_MSVC 0
#endif
#if !defined(COMPILER_GCC)
# define COMPILER_GCC 0
#endif
#if !defined(COMPILER_CLANG)
# define COMPILER_CLANG 0
#endif
#if !defined(OS_WINDOWS)
# define OS_WINDOWS 0
#endif
#if !defined(OS_LINUX)
# define OS_LINUX 0
#endif
#if !defined(OS_MAC)
# define OS_MAC 0
#endif

struct String{
  U8 *str;
  S64 len;
};

union Intern_String{  // Basically just String
  String s;
  struct{ U8 *str; S64 len; };
};

global String string_null = {(U8 *)"null", 4};
#include <stdio.h>
#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"
#define snprintf stbsp_snprintf


union Vec2 {
  struct { F32 x, y; };
  struct { F32 width, height; };
  F32 p[2];
};

union Vec3 {
  struct{ F32 x, y, z; };
  struct{ F32 r, g, b; };
  struct{ Vec2 xy; F32 z_; };
  struct{ F32 x_; Vec2 yz; };
  F32 p[3];
};

union Vec4 {
  struct{ F32 x, y, z, w; };
  struct{ F32 r, g, b, a; };
  struct{ Vec2 xy;  Vec2 zw;  };
  struct{ Vec2 xy_; F32 width, height; };
  struct{ Vec3 xyz; F32 w_; };
  struct{ F32 x_; Vec3 yzw; };
  struct{ Vec3 rgb; F32 a_; };
  F32 p[4];
};

struct Mat4 {
  F32 p[4][4];
};

union Vec1I {
  S32 x;
  S32 p[1];
};

union Vec2I {
  struct { S32 x, y; };
  struct { S32 width, height; };
  S32 p[2];
};

union Vec3I {
  struct { S32 x, y, z; };
  struct { S32 r, g, b; };
  struct { Vec2I xy; S32 z_; };
  struct { S32 x_; Vec2I yz; };
  S32 p[3];
};

union Vec4I {
  struct { S32 x, y, z, w; };
  struct { S32 r, g, b, a; };
  struct { Vec2I xy;  Vec2I zw; };
  struct { Vec2I xy_; S32 width, height; };
  struct { Vec3I xyz; S32 w_; };
  struct { S32 x_; Vec3I yzw; };
  struct { Vec3I rgb; S32 a_; };
  S32 p[4];
};

union Rect2 {
  struct {F32 min_x, min_y, max_x, max_y;};
  struct { Vec2 min; Vec2 max; };
  F32 p[4];
};

union Rect2I {
  struct { S32 min_x, min_y, max_x, max_y;};
  struct { Vec2I min; Vec2I max; };
  S32 p[4];
};

//-----------------------------------------------------------------------------
// Utilities
//-----------------------------------------------------------------------------
function SizeU
get_align_offset(SizeU size, SizeU align){
  SizeU mask = align - 1;
  SizeU val = size & mask;
  if(val){
    val = align - val;
  }
  return val;
}

function SizeU
align_up(SizeU size, SizeU align){
  SizeU result = size + get_align_offset(size, align);
  return result;
}

function SizeU
align_down(SizeU size, SizeU align){
  size += 1; // Make sure 8 when align is 8 doesn't get rounded down to 0
  SizeU result = size - (align - get_align_offset(size, align));
  return result;
}

function void
memory_copy(void *dst, void *src, SizeU size){
  U8 *d = (U8*)dst;
  U8 *s = (U8*)src;
  for(SizeU i = 0; i < size; i++){
    d[i] = s[i];
  }
}

function void
memory_zero(void *p, SizeU size){
  U8 *pp = (U8 *)p;
  for(SizeU i = 0; i < size; i++)
    pp[i] = 0;
}

template<class T>
void swap(T &a, T &b){
  T temp = a;
  a = b;
  b = temp;
}

template<class T>
T max(T a, T b){
  if(a > b) return a;
  return b;
}

template<class T>
T min(T a, T b){
  if(a > b) return b;
  return a;
}

template<class T>
T clamp_top(T val, T max){
  if(val > max) val = max;
  return val;
}

template<class T>
T clamp_bot(T bot, T val){
  if(val < bot) val = bot;
  return val;
}

template<class T>
T clamp(T min, T val, T max){
  if(val > max) val = max;
  if(val < min) val = min;
  return val;
}

function U64
hash_string(String string) {
  U64 hash = (U64)14695981039346656037ULL;
  for (U64 i = 0; i < string.len; i++) {
    hash = hash ^ (U64)(string.str[i]);
    hash = hash * (U64)1099511628211ULL;
  }
  return hash;
}

function U64
hash_u64(U64 x) {
  x *= 0xff51afd7ed558ccd;
  x ^= x >> 32;
  return x;
}

function U64
hash_ptr(const void *ptr) {
  return hash_u64((uintptr_t)ptr);
}

function U64
hash_mix(U64 x, U64 y) {
  // @note: murmur hash 3 mixer but I add the 'y'
  // which means it's probably bad, hopefully better
  // then some random scribble I could do
  x ^= (y >> 33);
  x *= 0xff51afd7ed558ccd;
  x ^= (x >> 33);
  x *= 0xc4ceb9fe1a85ec53;
  x ^= (y >> 33);
  return x;
}

function U64
is_pow2(U64 x) {
  assert(x != 0);
  B32 result = (x & (x - 1llu)) == 0;
  return result;
}

function U64
wrap_around_pow2(U64 x, U64 power_of_2) {
  assert(is_pow2(power_of_2));
  U64 r = (((x)&((power_of_2)-1llu)));
  return r;
}

force_inline String
operator""_s(const char *str, size_t size){
  return String{(U8 *)str, (S64)size};
}

force_inline B32
operator==(Intern_String a, Intern_String b){
  return a.str == b.str;
}

force_inline B32
operator!=(Intern_String a, Intern_String b){
  B32 result = a.str == b.str;
  return !result;
}

//-----------------------------------------------------------------------------
// Very cool macros. Since these are macros it's recommended to wrap them
// in a function and not use directly
//-----------------------------------------------------------------------------
#define SLLQueuePushMod(f, l, n, next)                                                   \
  do {                                                                                   \
    if ((f) == 0) {                                                                      \
      (f) = (l) = (n);                                                                   \
    } else {                                                                             \
      (l) = (l)->next = (n);                                                             \
    }                                                                                    \
  } while (0)
#define SLLQueuePush(f, l, n) SLLQueuePushMod(f, l, n, next)

#define SLLStackPush(l, n)                                                               \
  do {                                                                                   \
    (n)->next = (l);                                                                     \
    (l)       = (n);                                                                     \
  } while (0)

#define SLLStackPop(l, n)                                                                \
  do {                                                                                   \
    if (l) {                                                                             \
      (n)       = (l);                                                                   \
      (l)       = (l)->next;                                                             \
      (n)->next = 0;                                                                     \
    }                                                                                    \
  } while (0)

#define DLLQueuePushLastMod(f, l, node, next, prev)                                      \
  do {                                                                                   \
    if ((f) == 0) {                                                                      \
      (f) = (l)    = (node);                                                             \
      (node)->prev = 0;                                                                  \
      (node)->next = 0;                                                                  \
    } else {                                                                             \
      (l)->next    = (node);                                                             \
      (node)->prev = (l);                                                                \
      (node)->next = 0;                                                                  \
      (l)          = (node);                                                             \
    }                                                                                    \
  } while (0)
#define DLLQueuePushLast(f,l,node) DLLQueuePushLastMod(f,l,node,next,prev)
#define DLLQueuePush(f,l,node) DLLQueuePushLast(f,l,node)
#define DLLFreeListAddMod(first, node, next, prev)                                       \
  do {                                                                                   \
    (node)->next = (first);                                                              \
    if ((first))                                                                         \
      (first)->prev = (node);                                                            \
    (first)      = (node);                                                               \
    (node)->prev = 0;                                                                    \
  } while (0)
#define DLLFreeListAdd(first, node) DLLFreeListAddMod(first, node, next, prev)

#define For_List_It(a,it) for(auto *it = (a)->first; it; it=it->next) // @todo: reference?
#define For_List(a) For_List_It(a,it)
#define For_Named(a,it) for(auto &it : (a))
#define For(a) For_Named((a),it)

//-----------------------------------------------------------------------------
// Base Allocator stuff
//-----------------------------------------------------------------------------
enum Allocation_Kind{
  Allocation_Alloc,
  Allocation_Resize,
  Allocation_FreeAll,
  Allocation_Free,
  Allocation_Destroy
};

enum Allocator_Kind{
  Allocator_None,
  Allocator_Arena,
  Allocator_PersonalArena,
  Allocator_OSHeap,
};

enum Alloc_Flag{
  AF_None,
  AF_ZeroMemory
};

struct Allocator;
typedef void *Allocator_Proc(Allocator*, Allocation_Kind, void *, SizeU);
struct Allocator{Allocator_Kind kind; Allocator_Proc *proc; String debug_name;};

//-----------------------------------------------------------------------------
// Memory arenas
//-----------------------------------------------------------------------------
struct OS_Memory{
  SizeU commit, reserve;
  U8 *data;
};
function OS_Memory os_reserve(SizeU size);
function B32 os_commit(OS_Memory *m, SizeU size);
function void os_release(OS_Memory *m);
function B32 os_decommit_pos(OS_Memory *m, SizeU pos);

//-----------------------------------------------------------------------------
// Memory arenas
//-----------------------------------------------------------------------------
global const SizeU default_reserve_size   = gib(4);
global const SizeU default_alignment      = 8;
global const SizeU additional_commit_size = mib(1);
struct Arena:Allocator{
  OS_Memory memory;
  SizeU alignment;
  SizeU len;

  // Personal arena memes so we can compute correct size when resizing
  // Also a pointer so that we can make sure it didn't change
  SizeU old_size;
  void *debug_prev_pointer;
};
function void arena_init(Arena *arena, String debug_name);

function void
arena_pop_pos(Arena *arena, SizeU pos){
  pos = clamp_top(pos, arena->len);
  arena->len = pos;
}

function void *
arena_pop(Arena *arena, SizeU size){
  size = clamp_top(size, arena->len);
  arena->len -= size;
  return arena->memory.data + arena->len;
}

function void
arena_release(Arena *arena){
  os_release(&arena->memory);
}

function void
arena_clear(Arena *arena){
  arena_pop_pos(arena, 0);
}

function void *
arena_push_size(Arena *a, SizeU size){
  SizeU generous_size = size + a->alignment;
  if(a->len+generous_size>a->memory.commit){
    if(a->memory.reserve == 0){
      arena_init(a, "Zero initialized arena"_s);
    }
    B32 result = os_commit(&a->memory, generous_size+additional_commit_size);
    assert(result);
  }

  a->len = align_up(a->len, a->alignment);
  assert(a->memory.reserve > a->len + size);
  void *result = (U8*)a->memory.data + a->len;
  a->len += size;
  return result;
}

force_inline void *
arena_allocator_proc(Allocator *a, Allocation_Kind kind, void *old_pointer, SizeU size){
  Arena *arena = (Arena *)a;
  switch(kind){
    case Allocation_Alloc: return arena_push_size(arena, size);
    case Allocation_Resize:{
      void *result = arena_push_size(arena, size);
      memory_copy(result, old_pointer, size);
      return result;
    }
    case Allocation_Free : return 0;
    case Allocation_FreeAll: arena_clear(arena); return 0;
    case Allocation_Destroy: arena_release(arena); return 0;
  }
  invalid_codepath;
  return 0;
}

force_inline void *
personal_arena_allocator_proc(Allocator *a, Allocation_Kind kind, void *old_pointer, SizeU size){
  Arena *arena = (Arena *)a;
  arena->alignment = 1;

  void *result = 0;
  switch(kind){
    case Allocation_Resize: {
      assert(arena->old_size);
      assert(arena->old_size < size);
      assert(arena->debug_prev_pointer == old_pointer);
      result = arena_push_size(arena, size - arena->old_size);
      result = old_pointer;
    } break;
    default: {
      result = arena_allocator_proc(a, kind, old_pointer, size);
      arena->debug_prev_pointer = result;
    }
  }

  arena->old_size = size;
  return result;
}

function void
arena_init(Arena *a, String debug_name){
  a->memory     = os_reserve(default_reserve_size);
  a->alignment  = default_alignment;
  a->debug_name = debug_name;
  a->kind       = Allocator_Arena;
  if(!a->proc) a->proc = arena_allocator_proc;
}

function Arena
arena_make_personal(String debug_name){
  Arena arena = {};
  arena.proc = personal_arena_allocator_proc;
  arena_init(&arena, debug_name);
  arena.kind = Allocator_PersonalArena;
  return arena;
}

//-----------------------------------------------------------------------------
// OS Heap allocator
//-----------------------------------------------------------------------------
struct OS_Heap:Allocator{
  HANDLE handle;
};

function void *
os_heap_allocator_proc(Allocator *a, Allocation_Kind kind, void *old_pointer, SizeU size){
  OS_Heap *heap = (OS_Heap *)a;
  switch(kind){
    case Allocation_FreeAll:{
      invalid_codepath;
      return 0;
    }
    case Allocation_Destroy:{
      BOOL result = HeapDestroy(heap->handle);
      assert(result != 0);
      heap->handle = 0;
      heap->proc = 0;
      return 0;
    }
    case Allocation_Free:{
      BOOL result = HeapFree(heap->handle, 0, old_pointer);
      assert(result != 0);
      return 0;
    }
    case Allocation_Alloc:{
      void *result = HeapAlloc(heap->handle, 0, size);
      assert(result);
      return result;
    }
    case Allocation_Resize:{
      void *result = HeapReAlloc(heap->handle, 0, old_pointer, size);
      assert(result);
      return result;
    }
    default: invalid_codepath;
  }
  return 0;
}

function OS_Heap // max_size == 0 == growing heap
win32_os_heap_create(B32 multithreaded, SizeU initial_size, SizeU max_size, String debug_name){
  OS_Heap result = {};
  result.debug_name = debug_name;
  result.proc = os_heap_allocator_proc;
  result.kind = Allocator_OSHeap;
  result.handle = HeapCreate(multithreaded ? 0 : HEAP_NO_SERIALIZE, initial_size, max_size);
  assert(result.handle);
  return result;
}

enum Log_Kind{Log_Kind_Normal, Log_Kind_Error};
typedef void Log_Proc(Log_Kind kind, String string, char *file, int line);
//-----------------------------------------------------------------------------
// Thread Context
//-----------------------------------------------------------------------------
struct Thread_Ctx{
  Arena      scratch[2];
  Log_Proc  *log_proc;
  Allocator *implicit_alloc;

  int line;
  char *file;
};

thread_local Thread_Ctx thread_ctx;
global       Arena pernament_arena;
global       OS_Heap os_process_heap;

#define REPORT_ALLOCATIONS 0
#define report_file_and_line() report__file_and_line(__FILE__, __LINE__)
force_inline void
report__file_and_line(const char *file, int line){
  thread_ctx.file = (char *)file;
  thread_ctx.line =         line;
}

//-----------------------------------------------------------------------------
// Implicit scratch stack
//-----------------------------------------------------------------------------
struct Scratch{
  SizeU saved_pos;
  Arena *arena;

  Scratch(Allocator *conflict = 0){
    if(conflict == thread_ctx.scratch){
      arena = thread_ctx.scratch + 1;
    }
    else {
      arena = thread_ctx.scratch;
    }
    saved_pos = arena->len;
  }
  ~Scratch(){
    arena_pop_pos(arena, saved_pos);
  }
  force_inline operator Arena*(){ return arena; }
  force_inline operator Allocator*(){ return arena; }

  // @Note: Disable copy constructors, cause it caused lots of confusing errors
  //        Where it passed scratch instead of the arena into the constructor
  //        which is an error
  private:
  Scratch(Scratch &arena);
  Scratch(Scratch &arena, Scratch &a2);
};

#define Set_Allocator(a) Scoped_Allocator JOIN(scoped_alloc, __LINE__)(a)
struct Scoped_Allocator{
  Allocator *prev_allocator;
  Scoped_Allocator(Allocator *a){
    prev_allocator = thread_ctx.implicit_alloc;
    thread_ctx.implicit_alloc = a;
  }
  ~Scoped_Allocator(){
    thread_ctx.implicit_alloc = prev_allocator;
  }
};

//-----------------------------------------------------------------------------
// Explicit allocator
//-----------------------------------------------------------------------------
#define exp_alloc_array(a, T, size,...) (T *)exp_alloc(a, sizeof(T)*(size),##__VA_ARGS__)
#define exp_alloc_type(a, T, ...) exp_alloc_array(a, T, 1,##__VA_ARGS__)
#define exp_alloc(a, size, ...) (report_file_and_line(), exp__alloc(a, size,##__VA_ARGS__))
#define exp_resize(a,p,size) (report_file_and_line(), exp__resize(a, p, size))
#define exp_resize_array(a, p, T, size) (report_file_and_line(), (T *)exp_resize(a, p, sizeof(T)*(size)))
#define exp_free(a, p) (report_file_and_line(), exp__free(a, p))
#define exp_free_all(a) (report_file_and_line(), exp__free_all(a))
#define exp_destroy(a) (report_file_and_line(), exp__destroy(a))

force_inline void *
exp__alloc(Allocator *a, SizeU size, Alloc_Flag flag = AF_None){
  #if REPORT_ALLOCATIONS
  printf("Alloc(%s) %s:%d %u\n", a->debug_name.str, thread_ctx.file, thread_ctx.line, (U32)size);
  #endif

  void *result = a->proc(a, Allocation_Alloc, 0, size);
  if(flag & AF_ZeroMemory) memory_zero(result, size);
  return result;
}
force_inline void *
exp__resize(Allocator *a, void *pointer, SizeU size){
  #if REPORT_ALLOCATIONS
  printf("Resize(%s) %s:%d %u\n", a->debug_name.str, thread_ctx.file, thread_ctx.line, (U32)size);
  #endif

  return a->proc(a, Allocation_Resize, pointer, size);
}
force_inline void
exp__free(Allocator *a, void *pointer){
  #if REPORT_ALLOCATIONS
  printf("Free(%s) %s:%d\n", a->debug_name.str, thread_ctx.file, thread_ctx.line);
  #endif

  a->proc(a, Allocation_Free, pointer, 0);
}
force_inline void
exp__free_all(Allocator *a){
  #if REPORT_ALLOCATIONS
  printf("FreeAll(%s) %s:%d\n", a->debug_name.str, thread_ctx.file, thread_ctx.line);
  #endif

  a->proc(a, Allocation_FreeAll, 0, 0);
}
force_inline void
exp__destroy(Allocator *a){
  #if REPORT_ALLOCATIONS
  printf("Destroy(%s) %s:%d\n", a->debug_name.str, thread_ctx.file, thread_ctx.line);
  #endif

  a->proc(a, Allocation_Destroy, 0, 0);
}
force_inline Allocator *
imp_get(){
  assert(thread_ctx.implicit_alloc);
  return thread_ctx.implicit_alloc;
}

function void
thread_ctx_init(){
  arena_init(thread_ctx.scratch, "Scratch1"_s);
  arena_init(thread_ctx.scratch+1, "Scratch2"_s);
  arena_init(&pernament_arena, "Pernament Arena"_s);
  os_process_heap.proc             = os_heap_allocator_proc;
  os_process_heap.handle           = GetProcessHeap();
  os_process_heap.debug_name       = "Win32 Process Heap"_s;
  os_process_heap.kind             = Allocator_OSHeap;
}

#include "base_string.cpp"
//-----------------------------------------------------------------------------
// Logging
//-----------------------------------------------------------------------------
#define log_info(...) handle_log_message(Log_Kind_Normal, __LINE__, __FILE__,##__VA_ARGS__)
#define log_error(...) handle_log_message(Log_Kind_Error, __LINE__, __FILE__,##__VA_ARGS__)
function void
handle_log_message(Log_Kind kind, int line, const char *file, const char *str, ...){
  Scratch scratch;
  STRING_FMT(scratch, str, message);
  if(thread_ctx.log_proc) thread_ctx.log_proc(kind, message, (char *)file, line);
  else{
    printf("%s", message.str);
  }
}

//-----------------------------------------------------------------------------
// Defer
// http://www.gingerbill.org/article/2015/08/19/defer-in-cpp/
//-----------------------------------------------------------------------------
template <typename F>
struct Defer_Scope {
  F f;
  Defer_Scope(F f) : f(f) {}
  ~Defer_Scope() { f(); }
};

template <typename F>
Defer_Scope<F> defer_func(F f) {
  return Defer_Scope<F>(f);
}
#define DEFER_1(x, y) x##y
#define DEFER_2(x, y) DEFER_1(x, y)
#define DEFER_3(x)    DEFER_2(x, __COUNTER__)
#define defer(code)   auto DEFER_3(_defer_) = defer_func([&](){code;})

//-----------------------------------------------------------------------------
// Array
//-----------------------------------------------------------------------------
template<class T>
struct Array{
  Allocator *allocator;
  T  *data;
  S64 cap;
  S64 len;

  T *push_empty(S64 count = 1){
    grow(count);
    T *result = data + len;
    len += count;
    return result;
  }

  T *push_empty_zero(S64 count = 1){
    T *result = push_empty(count);
    memory_zero(result, count*sizeof(T));
    return result;
  }

  void grow(S64 required_size){
    if(cap == 0){
      if(!allocator) allocator = imp_get();
      S64 new_cap = max(required_size*2, (S64)16);
      data = exp_alloc_array(allocator, T, new_cap);
      cap = new_cap;
    }
    else if(len + required_size > cap){
      U64 new_cap = max(cap * 2, len+required_size+1);
      data = exp_resize_array(allocator, data, T, new_cap);
      cap = new_cap;
    }
  }

  S64 get_index(T *item){
    assert((data <= item) && ((data + len) > item));
    SizeU offset = item - data;
    return (S64)offset;
  }

  void add(Array<T> items){
    For(items){
      add(it);
    }
  }

  void add(T item){
    grow(1);
    data[len++] = item;
  }

  S64 addi(T item){
    S64 result = len;
    grow(1);
    data[len++] = item;
    return result;
  }

  void unordered_remove(T *item){
    assert(len > 0);
    assert((data <= item) && ((data + len) > item));
    *item = data[--len];
  }

  void init(Allocator *a, S64 size = 16){
    allocator = a;
    data      = exp_alloc_array(a, T, size);
    cap       = size;
  }

  Array<T> copy(Allocator *a){
    Array<T> result = {};
    result.len = len;
    result.cap = len*2;
    result.allocator = a;
    result.data = exp_alloc_array(a, T, result.cap);
    memory_copy(result.data, data, sizeof(T)*result.len);
    return result;
  }

  Array<T> tight_copy(Allocator *a){
    Array<T> result = {};
    result.len = len;
    result.cap = len;
    result.allocator = 0;
    result.data = exp_alloc_array(a, T, len);
    memory_copy(result.data, data, sizeof(T)*len);
    return result;
  }

  force_inline B32 is_last(T *item){ return item == last();   }
  force_inline B32 is_first(T *item){ return item == begin(); }
  force_inline void clear(){ len = 0;                         }
  force_inline T pop()     { return data[--len];              }
  force_inline T *last()   { return data + len - 1;           }
  force_inline T *begin()  { return data;                     }
  force_inline T *end  ()  { return data + len;               }
  force_inline T &operator[](S64 i){ assert(i >= 0 && i < cap); return data[i]; }


  struct Array_Iter{
    Array<T> *array;
    S64 i;
    T *it;

    force_inline void next(){ i+=1; it = &array->data[i]; }
    force_inline B32 should_continue(){ return i < array->len; }
  };

  force_inline Array_Iter iter(){ return {this, 0, begin()};}
  #define For_It_Named(array, it) for(auto it = (array).iter(); it.should_continue(); it.next())
  #define For_It(array) For_It_Named(array, it)
};


template<class T>
function Array<T>
array_make(Allocator *a, S64 size = 16){
  Array<T> result = {};
  result.init(a, size);
  return result;
}

function void
test_array(){
  Scratch scratch;
  Array<int> array = {scratch};

  int size = 1000;
  for(int i = 0; i < size; i++){
    array.add(i);
  }
  S32 i = 0;
  For(array){
    assert(it == i++);
  }

  Arena arena = arena_make_personal("Test personal arena"_s);
  Array<int> array2 = {&arena};
  for(int i = 0; i < size; i++){
    array2.add(i);
  }
  i=0;
  For(array2){
    assert(it == i++);
  }
  exp_destroy(&arena);
  assert(arena.memory.data == 0);
  assert(thread_ctx.scratch->memory.data != 0);
}


template<class T, S64 bucket_size>
struct Simple_Bucket_Array{
  struct Bucket{ Bucket *next; T data[bucket_size]; S64 len; };
  Bucket *first;
  Bucket *last ;

  void grow(Allocator *allocator){
    if(!first){
      Bucket *bucket = exp_alloc_type(allocator, Bucket);
      bucket->next = 0; bucket->len  = 0;
      first = last = bucket;
    }

    if(last->len >= bucket_size){
      Bucket *bucket = exp_alloc_type(allocator, Bucket);
      bucket->next = 0; bucket->len  = 0;
      last = last->next = bucket;
    }
  }

  T *allocate(Allocator *allocator){
    grow(allocator);
    return last->data + last->len++;
  }

  void add(Allocator *allocator, T item){
    grow(allocator);
    last->data[last->len++] = item;
  }

  struct Iter{
    Simple_Bucket_Array *ref;
    Bucket *bucket;
    T *it;
    S64 iter_i;
    S64 i;

    force_inline B32 should_continue(){ return it != 0; }
    force_inline void next(){
      if(iter_i >= bucket->len){
        if(bucket->next){
          bucket = bucket->next;
          iter_i = 0;
        }
        else{
          it = 0;
          return;
        }
      }

      it = bucket->data + iter_i++;
      i += 1;
    }
  };

  force_inline Iter iter(){
    if(!this->first || this->first->len == 0) return {};
    return {this, this->first, this->first->data, 1};
  }
};

function void
test_bucket_arrays(){
  Scratch scratch;
  Simple_Bucket_Array<int, 4> a = {};
  for(int i = 0; i < 32; i++)
    a.add(scratch, i);

  For_It(a){
    assert(*it.it == it.i);
  }

  for(auto i = a.iter(); i.should_continue(); i.next()){
    assert(*i.it == i.i);
  }

  int total_i = 0;
  for(auto bucket = a.first; bucket; bucket=bucket->next){
    for(int i = 0; i < bucket->len; i++){
      assert(bucket->data[i] == total_i++);
    }
  }
}

//-----------------------------------------------------------------------------
// Map
//-----------------------------------------------------------------------------
struct Map_Key_Value{
  int occupied;
  U64   key;
  void *value;
};

struct Map{
  Allocator *allocator;
  Map_Key_Value *data;
  S64 len;
  S64 cap;
};
function void map_insert(Map *map, U64 key, void *val);

function void
map_grow(Map *map, S64 new_size){
  new_size = max((S64)16, new_size);
  assert(new_size > map->cap);
  assert(is_pow2(new_size));
  if(map->cap == 0 && map->allocator == 0) map->allocator = imp_get();

  Map new_map       = {};
  new_map.data      = exp_alloc_array(map->allocator, Map_Key_Value, new_size, AF_ZeroMemory);
  new_map.cap       = new_size;
  new_map.allocator = map->allocator;

  for(S64 i = 0; i < map->cap; i++){
    if(map->data[i].occupied){
      map_insert(&new_map, map->data[i].key, map->data[i].value);
    }
  }
  if(map->data) exp_free(map->allocator, map->data);
  *map = new_map;
}

function Map
map_make(Allocator *a, S64 size){
  Map result = {a};
  map_grow(&result, size);
  return result;
}

function void
map_insert(Map *map, U64 key, void *val){
  assert(val);
  assert(key);
  // if(key == 0) key+=1;

  if((2*map->len) + 1 > map->cap){
    map_grow(map, 2*map->cap);
  }

  U64 hash = hash_u64(key);
  U64 index = wrap_around_pow2(hash, map->cap);
  U64 i     = index;
  for(;;){
    if(map->data[i].occupied == false){
      map->len++;
      map->data[i].occupied = true;
      map->data[i].key      = key;
      map->data[i].value    = val;
      return;
    }
    else if(map->data[i].key == key){
      map->data[i].value = val;
      return;
    }

    i = wrap_around_pow2(i+1, map->cap);
    if(i == map->cap){
      return;
    }
  }
}

function Map_Key_Value *
map_base_get(Map *map, U64 key){
  if(map->len == 0) return 0;
  assert(key);
  // if(key == 0) key+=1;

  U64 hash = hash_u64(key);
  U64 index = wrap_around_pow2(hash, map->cap);
  U64 i     = index;
  for(;;){
    if(map->data[i].key == key){
      return map->data + i;
    }
    else if(map->data[i].key == 0){
      return 0;
    }

    i = wrap_around_pow2(i+1, map->cap);
    if(i == map->cap){
      return 0;
    }
  }
}

function void *
map_get(Map *map, U64 key){
  Map_Key_Value *result = map_base_get(map, key);
  if(result && result->occupied) return result->value;
  return 0;
}

function void *
map_remove(Map *map, U64 key){
  Map_Key_Value *kv = map_base_get(map, key);
  if(kv){
    kv->occupied = false;
    return kv->value;
  }
  return 0;
}

function void *
map_get(Map *map, void *pointer){
  return map_get(map, (U64)pointer);
}

function void *
map_get(Map *map, Intern_String string){
  return map_get(map, hash_string(string.s));
}

function void
map_insert(Map *map, void *key, void *value){
  map_insert(map, (U64)key, value);
}

function void
map_insert(Map *map, Intern_String key, void *value){
  map_insert(map, hash_string(key.s), value);
}

function void
map_test(){
  Scratch scratch;
  Map map = {scratch};
  const SizeU size = 1025;
  for(SizeU i = 1; i < size; i++){
    map_insert(&map, i, (void *)i);
  }
  for(SizeU i = 1; i < size; i++){
    SizeU val = (SizeU)map_get(&map, i);
    assert(val == i);
  }
}

//-----------------------------------------------------------------------------
// String intern
//-----------------------------------------------------------------------------
struct Intern_Table{
  Allocator *string_allocator;
  Map map;
  U8 *first_keyword;
  U8 *last_keyword;
};

function Intern_Table
intern_table_make(Allocator *string_allocator, Allocator *map_allocator, S64 initial_size = 32){
  Intern_Table result     = {};
  result.map              = map_make(map_allocator, initial_size);
  result.string_allocator = string_allocator;
  return result;
}

function Intern_String
intern_string(Intern_Table *t, String string){
  assert(t->string_allocator);
  U64 hash = hash_string(string);
  U8 *slot = (U8 *)map_get(&t->map, hash);
  if(slot){
    Intern_String result = {{slot, *(slot-sizeof(S64))}};
    return result;
  }

  S64 *len_address = (S64 *)exp_alloc(t->string_allocator, string.len+1+sizeof(S64));
  *len_address = string.len;

  U8 *string_address = (U8 *)(len_address + 1);
  memory_copy(string_address, string.str, string.len);
  string_address[string.len] = 0;

  map_insert(&t->map, hash, string_address);
  Intern_String result = {{string_address, *len_address}};

  return result;
}

function void
test_intern_table(){
  Scratch scratch;
  Intern_Table table = intern_table_make(scratch, scratch);
  Intern_String intern1 = intern_string(&table, "Thing"_s);
  Intern_String intern2 = intern_string(&table, "Thing"_s);
  Intern_String intern3 = intern_string(&table, "Not Thing"_s);
  assert(intern1.str == intern2.str);
  assert(intern3.str != intern2.str);
}

function Arena
arena_sub(Allocator *base, SizeU size, String debug_name) {
  Arena result = {};
  result.memory.data = (U8 *)exp_alloc(base, size);
  result.memory.commit = size;
  result.memory.reserve = size;
  result.alignment = default_alignment;
  result.len = 0;
  result.debug_name = debug_name;
  result.kind       = Allocator_Arena;
  if(!result.proc) result.proc = arena_allocator_proc;
  return result;
}
