#pragma once

struct Bitmap {
  union {
    U32 *pixels;
    U64 id;
  };
  union {
    Vec2I size;
    struct {
      S32 x, y;
    };
  };
  Vec2 align;
};

enum EventKind {
  EventKind_None,
  EventKind_KeyDown,
  EventKind_KeyUp,
  EventKind_MouseMove,
  EventKind_MouseWheel,
  EventKind_KeyboardText,
};

enum Key {
  Key_None,
  Key_Up,
  Key_Down,
  Key_Left,
  Key_Right,
  Key_Escape,
  Key_F1,
  Key_F2,
  Key_F3,
  Key_F4,
  Key_F5,
  Key_F6,
  Key_F7,
  Key_F8,
  Key_F9,
  Key_F10,
  Key_F11,
  Key_F12,
  Key_MouseLeft,
  Key_MouseRight,
  Key_MouseMiddle,
  Key_0 = '0',
  Key_1,
  Key_2,
  Key_3,
  Key_4,
  Key_5,
  Key_6,
  Key_7,
  Key_8,
  Key_9 = '9',
  Key_A = 'a',
  Key_B,
  Key_C,
  Key_D,
  Key_E,
  Key_F,
  Key_G,
  Key_H,
  Key_I,
  Key_J,
  Key_K,
  Key_L,
  Key_M,
  Key_N,
  Key_O,
  Key_P,
  Key_Q,
  Key_R,
  Key_S,
  Key_T,
  Key_U,
  Key_V,
  Key_W,
  Key_X,
  Key_Y,
  Key_Z = 'z',
  Key_Count = 256,
};

#define KEY_MAPPING                                                                                          \
X(Up, VK_UP)                                                                                               \
X(Down, VK_DOWN)                                                                                           \
X(Left, VK_LEFT)                                                                                           \
X(Right, VK_RIGHT)                                                                                         \
X(Escape, VK_ESCAPE)                                                                                       \
X(F1, VK_F1)                                                                                               \
X(F2, VK_F2)                                                                                               \
X(F3, VK_F3)                                                                                               \
X(F4, VK_F4)                                                                                               \
X(F5, VK_F5)                                                                                               \
X(F6, VK_F6)                                                                                               \
X(F7, VK_F7)                                                                                               \
X(F8, VK_F8)                                                                                               \
X(F9, VK_F9)                                                                                               \
X(F10, VK_F10)                                                                                             \
X(F11, VK_F11)                                                                                             \
X(F12, VK_F12)                                                                                             \
X(A, 65)                                                                                                   \
X(B, 66)                                                                                                   \
X(C, 67)                                                                                                   \
X(D, 68)                                                                                                   \
X(E, 69)                                                                                                   \
X(F, 70)                                                                                                   \
X(G, 71)                                                                                                   \
X(H, 72)                                                                                                   \
X(I, 73)                                                                                                   \
X(J, 74)                                                                                                   \
X(K, 75)                                                                                                   \
X(L, 76)                                                                                                   \
X(M, 77)                                                                                                   \
X(N, 78)                                                                                                   \
X(O, 79)                                                                                                   \
X(P, 80)                                                                                                   \
X(Q, 81)                                                                                                   \
X(R, 82)                                                                                                   \
X(S, 83)                                                                                                   \
X(T, 84)                                                                                                   \
X(U, 85)                                                                                                   \
X(V, 86)                                                                                                   \
X(W, 87)                                                                                                   \
X(X, 88)                                                                                                   \
X(Y, 89)                                                                                                   \
X(Z, 90)                                                                                                   \
X(0, 48)                                                                                                   \
X(1, 49)                                                                                                   \
X(2, 50)                                                                                                   \
X(3, 51)                                                                                                   \
X(4, 52)                                                                                                   \
X(5, 53)                                                                                                   \
X(6, 54)                                                                                                   \
X(7, 55)                                                                                                   \
X(8, 56)                                                                                                   \
X(9, 57)

struct DigitalKey {
  bool pressed;
  bool down;
  bool released;
};

enum RenderBackend {
  RenderBackend_Software,
  RenderBackend_OpenGL1,
};

struct OS {
  bool quit;
  bool initialized;
  Arena *frame_arena;
  Arena *perm_arena;
  
  F64 ms_per_frame;
  bool window_resizable;
  bool window_was_resized;
  String window_title;
  Vec2I window_size;
  Vec2I window_pos;
  RenderBackend render_backend;
  
  Bitmap *screen;
  Vec2I monitor_size;
  F32 dpi_scale;
  
  F64 fps;
  F64 delta_time;
  F64 time;
  U64 frame;
  F64 update_time;
  U64 update_begin_cycles;
  U64 update_end_cycles;
  F64 frame_start_time;
  F64 app_start_time;
  
  B32 cursor_visible;
  DigitalKey key[Key_Count];
  F32 mouse_wheel;
  Vec2I mouse_pos;
  Vec2I delta_mouse_pos;
  
  String text;
  
  char platform[256];
  struct {
    char *vendor;
    char *version;
    char *renderer;
    char *extensions;
  } opengl;
};

struct FontGlyph {
  F32 xadvance;
  U32 codepoint;
  Bitmap bitmap;
};

struct Font {
  F32 line_advance;
  F32 ascent;
  F32 descent;
  F32 height;
  FontGlyph *glyphs;
  U32 glyphs_len;
};

struct Audio;
#define AUDIO_CALLBACK(name) void name(Audio *audio, U32* buffer, U32 frames_to_fill)
typedef AUDIO_CALLBACK(AudioCallback);

struct Audio {
  AudioCallback *callback;
  B32 initialized;
  U32 samples_per_second;
  U32 number_of_channels;
  
  // NOTE: one frame is 2 samples (left, right) 32 bits if
  // one sample is equal 16 bit
  U32 buffer_frame_count;
  U32 latency_frame_count;
  S32 bits_per_sample;
  
  char platform[128];
};
