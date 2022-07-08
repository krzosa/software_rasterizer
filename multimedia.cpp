#include "base.cpp"
#include "base_unicode.cpp"
#include "os_windows.cpp"

#include <Windows.h>
#include <shellscalingapi.h>
#include <GL/Gl.h>
#include <intrin.h>

#pragma comment(linker, "/subsystem:windows")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "Shcore.lib")
#pragma comment(lib, "opengl32.lib")

// Symbols taken from GLFW
//
// Executables (but not DLLs) exporting this symbol with this value will be
// automatically directed to the high-performance GPU on Nvidia Optimus systems
// with up-to-date drivers
//
__declspec(dllexport) DWORD NvOptimusEnablement = 1;

// Executables (but not DLLs) exporting this symbol with this value will be
// automatically directed to the high-performance GPU on AMD PowerXpress systems
// with up-to-date drivers
//
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;

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

OS os;
#include "base_math.cpp"

typedef HRESULT tSetProcessDpiAwareness(PROCESS_DPI_AWARENESS);
typedef MMRESULT TimeBeginPeriod(MMRESULT);
constexpr DWORD window_style_simplified = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
constexpr DWORD window_style_resizable = WS_OVERLAPPEDWINDOW;
constexpr DWORD window_style_borderless = WS_POPUP;

struct Win32Bitmap {
  Bitmap bitmap;
  HDC dc;
  HBITMAP dib;
};

struct Win32FontCtx {
  HFONT font;
  Win32Bitmap bitmap;
  TEXTMETRIC text_metric;
  Font result;
};

struct OS_Win32 {
  HWND window;
  HDC window_dc;
  Win32Bitmap screen;
  HINSTANCE instance;
  int show_cmd;
  char *cmd_line;
  bool good_scheduling;
  void *main_fiber;
  void *msg_fiber;
  U8 text_buff[32];
  Vec2I prev_window_size;
  Vec2I prev_mouse_pos;
};
static_assert(sizeof(OS_Win32) < 256, "Too big");
#define w32(a) (*(OS_Win32 *)(a).platform)

api Bitmap
bitmap(U32 *pixels, Vec2I size, Vec2 align_in_pixels = {}) {
  Bitmap result;
  result.pixels = pixels;
  result.size = size;
  result.align = align_in_pixels;
  return result;
}

function Win32Bitmap
win32_create_bitmap(Vec2I size) {
  Win32Bitmap result;
  result.bitmap.size = size;
  if (result.bitmap.size.y < 0)
    result.bitmap.size.y = -result.bitmap.size.y;

  HDC hdc = GetDC(0);
  BITMAPINFO bminfo = {};
  bminfo.bmiHeader.biSize = sizeof(bminfo.bmiHeader);
  bminfo.bmiHeader.biWidth = (LONG)size.x;
  bminfo.bmiHeader.biHeight = (LONG)-size.y;
  bminfo.bmiHeader.biPlanes = 1;
  bminfo.bmiHeader.biBitCount = 32;
  bminfo.bmiHeader.biCompression = BI_RGB; // AA RR GG BB
  bminfo.bmiHeader.biXPelsPerMeter = 1;
  bminfo.bmiHeader.biYPelsPerMeter = 1;

  void *mem = 0;
  result.dib = CreateDIBSection(hdc, &bminfo, DIB_RGB_COLORS, (void **)&mem, 0, 0);
  assert_msg(mem != 0, "Failed to create win32 bitmap");
  result.dc = CreateCompatibleDC(hdc);
  result.bitmap.pixels = (U32 *)mem;
  return result;
}

function void
win32_destroy_bitmap(Win32Bitmap *b) {
  if (b->bitmap.pixels) {
    b->bitmap.pixels = 0;
    DeleteDC(b->dc);
    DeleteObject(b->dib);
  }
}

function void
win32_write_user_text(U8 *text, S32 len){
  if(os.text.len+len < buff_cap(w32(os).text_buff)){
    for(S32 i = 0; i < len; i++){
      os.text.str[os.text.len++] = text[i];
    }
  }
}

function LRESULT
CALLBACK win32_window_proc(HWND window, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  EventKind kind = EventKind_None;
  Key key = Key_None;
  LRESULT result = 0;
  switch (uMsg) {
    case WM_CLOSE:
    DestroyWindow(window);
    os.quit = 1;
    break;
    case WM_DESTROY:
    PostQuitMessage(0);
    os.quit = 1;
    break;
    case WM_SYSKEYDOWN:
    case WM_KEYDOWN: {
      kind = EventKind_KeyDown;
      switch (wParam) {

#define X(button, win)                                                                                       \
case win:                                                                                                  \
key = Key_##button;                                                                                      \
break;

        KEY_MAPPING
      }
    } break;
    case WM_SYSKEYUP:
    case WM_KEYUP: {
      kind = EventKind_KeyUp;
      switch (wParam) {
        KEY_MAPPING
#undef X
      }
    } break;
    case WM_TIMER: {
      SwitchToFiber(w32(os).main_fiber);
    } break;
    case WM_ENTERMENULOOP:
    case WM_ENTERSIZEMOVE: {
      SetTimer(w32(os).window, 0, 1, 0);
    } break;
    case WM_EXITMENULOOP:
    case WM_EXITSIZEMOVE: {
      KillTimer(w32(os).window, 0);
    } break;
    case WM_MOUSEMOVE: {
      POINT p;
      GetCursorPos(&p);
      ScreenToClient(w32(os).window, &p);
      os.mouse_pos = vec2i(p.x, p.y);
      os.mouse_pos.y = os.window_size.y - os.mouse_pos.y;
      kind = EventKind_MouseMove;
    } break;
    case WM_LBUTTONDOWN: {
      kind = EventKind_KeyDown;
      key = Key_MouseLeft;
    } break;
    case WM_LBUTTONUP: {
      kind = EventKind_KeyUp;
      key = Key_MouseLeft;
    } break;
    case WM_RBUTTONDOWN: {
      kind = EventKind_KeyDown;
      key = Key_MouseRight;
    } break;
    case WM_RBUTTONUP: {
      kind = EventKind_KeyUp;
      key = Key_MouseRight;
    } break;
    case WM_MBUTTONDOWN: {
      kind = EventKind_KeyDown;
      key = Key_MouseMiddle;
    } break;
    case WM_MBUTTONUP: {
      kind = EventKind_KeyUp;
      key = Key_MouseMiddle;
    } break;
    case WM_MOUSEWHEEL: {
      if ((int)wParam > 0)
        os.mouse_wheel = 1;
      else
        os.mouse_wheel = -1;
      kind = EventKind_MouseWheel;
    } break;
    case WM_CHAR: {
      kind = EventKind_KeyboardText;
      // @Note(Krzosa): No surrogate extensions
      UTF32_Result decode = utf16_to_utf32((U16 *)&wParam, 1);
      if(!decode.error){
        UTF8_Result encode = utf32_to_utf8(decode.out_str);
        if(!encode.error){
          win32_write_user_text(encode.out_str, encode.len);
        }
        else win32_write_user_text((U8 *)"?", 1);
      }
      else win32_write_user_text((U8 *)"?", 1);
    } break;
    default:
    result = DefWindowProcW(window, uMsg, wParam, lParam);
    break;
  }

  if (kind) {
    if (kind == EventKind_KeyDown) {
      if (os.key[key].down == 0)
        os.key[key].pressed = 1;
      os.key[key].down = 1;
    } else if (kind == EventKind_KeyUp) {
      os.key[key].released = 1;
      os.key[key].down = 0;
    }
  }

  return result;
}

function void
CALLBACK _os_fiber_event_proc(void *data) {
  unused(data);
  for (;;) {
    MSG msg;
    while (PeekMessageW(&msg, w32(os).window, 0, 0, PM_REMOVE)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
    SwitchToFiber(w32(os).main_fiber);
  }
}

function Vec2I
get_window_size(HWND window) {
  Vec2I result;
  RECT window_rect;
  GetClientRect(window, &window_rect);
  result.x = window_rect.right - window_rect.left;
  result.y = window_rect.bottom - window_rect.top;
  return result;
}

function Vec2I
get_window_size_with_border(HWND window) {
  RECT ClientRect;
  GetWindowRect(window, &ClientRect);
  Vec2I draw_area;
  draw_area.x = (ClientRect.right - ClientRect.left);
  draw_area.y = (ClientRect.bottom - ClientRect.top);
  return draw_area;
}

function Vec2I
get_border_size(HWND window) {
  Vec2I client = get_window_size(window);
  Vec2I wind_size = get_window_size_with_border(window);
  Vec2I result = vec2i(wind_size.x - client.x, wind_size.y - client.y);
  return result;
}

api void os_quit() { os.quit = 1; }
api void os_set_fps(F64 fps) { os.ms_per_frame = 1 / fps; }
api void os_set_ms_per_frame(F64 ms) { os.ms_per_frame = ms; }

api void os_show_cursor(B32 status) {
  ShowCursor(status);
  os.cursor_visible = status;
}

api void os_set_window_title(String title) {
  Scratch scratch;
  BOOL result = SetWindowTextA(w32(os).window, (char *)title.str);
  assert_msg(result != 0, "Failed to set window title");
  os.window_title = title;
}

api void os_set_window_size(S32 x, S32 y) {
  Vec2I border = get_border_size(w32(os).window);
  int actual_width = (int)(x + border.x);
  int actual_height = (int)(y + border.y);
  bool result =
    SetWindowPos(w32(os).window, 0, 0, 0, actual_width, actual_height, SWP_NOMOVE | SWP_NOOWNERZORDER);
  assert_msg(result, "SetWindowPos returned invalid value");
  os.window_size = get_window_size(w32(os).window);
}

api void os_pull_state() {
  os.window_was_resized = false;
  os.window_size = get_window_size(w32(os).window);
  if (os.window_size != w32(os).prev_window_size) {
    os.window_was_resized = true;
    w32(os).prev_window_size = os.window_size;
  }
  os.monitor_size = vec2i(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));

  // @Note: Client relative
  POINT point = {0, 0};
  ClientToScreen(w32(os).window, &point);
  os.window_pos.x = point.x;
  os.window_pos.y = point.y;

  // @Note: Get DPI scale
  UINT dpi = GetDpiForWindow(w32(os).window);
  assert_msg(dpi != 0, "Failed to get dpi for window");
  os.dpi_scale = (F32)dpi / 96.f;

  // @Note: Reset text
  os.text.len = 0;
  // @Note: Reset keys
  for (int i = 0; i < Key_Count; i++) {
    os.key[i].released = 0;
    os.key[i].pressed = 0;
  }
  os.mouse_wheel = 0;
  SwitchToFiber(w32(os).msg_fiber);
  if (!os.quit) {
    os.delta_mouse_pos = w32(os).prev_mouse_pos - os.mouse_pos;
    w32(os).prev_mouse_pos = os.mouse_pos;

    // @Note: Resize
    if (os.render_backend == RenderBackend_Software) {
      if (os.window_size != w32(os).screen.bitmap.size && os.window_size.x != 0 && os.window_size.y != 0) {
        win32_destroy_bitmap(&w32(os).screen);
        w32(os).screen = win32_create_bitmap(vec2i(os.window_size.x, -(os.window_size.y)));
        os.screen = &w32(os).screen.bitmap;
      }
    }
  }
}

api void os_init_software_render() { os.render_backend = RenderBackend_Software; }

api B32 os_init_opengl() {
  PIXELFORMATDESCRIPTOR p = {};
  p.nSize = sizeof(p);
  p.nVersion = 1;
  p.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
  p.iPixelType = PFD_TYPE_RGBA;
  p.cColorBits = 32;
  p.cDepthBits = 24;
  p.cStencilBits = 8;
  p.iLayerType = PFD_MAIN_PLANE;

  S32 pixel_format = ChoosePixelFormat(w32(os).window_dc, &p);
  if (pixel_format != 0) {
    if (SetPixelFormat(w32(os).window_dc, pixel_format, &p)) {
      HGLRC gl_ctx = wglCreateContext(w32(os).window_dc);
      if (gl_ctx != NULL) {
        if (wglMakeCurrent(w32(os).window_dc, gl_ctx)) {
          // Success
        }
        else {
          log_error("Failed on wglMakeCurrent!");
          return false;
        }
      }
      else {
        log_error("Failed on wglCreateContext!");
        return false;
      }
    }
    else {
      log_error("Failed on SetPixelFormat!");
      return false;
    }
  }
  else {
    log_error("Failed on ChoosePixelFormat!");
    return false;
  }

  os.opengl.vendor = (char *)glGetString(GL_VENDOR);
  os.opengl.renderer = (char *)glGetString(GL_RENDERER);
  os.opengl.version = (char *)glGetString(GL_VERSION);
  os.opengl.extensions = (char *)glGetString(GL_EXTENSIONS);
  os.render_backend = RenderBackend_OpenGL1;

  return true;
}

api B32 os_init() {
  HMODULE shcore = LoadLibraryA("Shcore.dll");
  if (shcore) {
    tSetProcessDpiAwareness *set_dpi_awr =
    (tSetProcessDpiAwareness *)GetProcAddress(shcore, "SetProcessDpiAwareness");
    if (set_dpi_awr) {
      HRESULT hr = set_dpi_awr(PROCESS_PER_MONITOR_DPI_AWARE);
      assert_msg(SUCCEEDED(hr), "Failed to set dpi awareness");
    }
  }

  HMODULE winmm = LoadLibraryA("winmm.dll");
  if (winmm) {
    TimeBeginPeriod *timeBeginPeriod = (TimeBeginPeriod *)GetProcAddress(winmm, "timeBeginPeriod");
    if (timeBeginPeriod) {
      if (timeBeginPeriod(1) == TIMERR_NOERROR) {
        w32(os).good_scheduling = true;
      }
    }
  }

  DWORD window_style_chosen = window_style_resizable;
  if (!os.window_resizable)
    window_style_chosen = window_style_simplified;

  os.app_start_time = os_time();
  os.frame_start_time = os.app_start_time;
  os.update_begin_cycles = __rdtsc();

  Scratch scratch;
  WNDCLASSW wc = {};
  wc.lpfnWndProc = win32_window_proc;
  wc.hInstance = w32(os).instance;
  wc.lpszClassName = (LPCWSTR)string8_to_string16(scratch, os.window_title).str;
  wc.hCursor = LoadCursor(0, IDC_ARROW);
  wc.style = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
  if (!RegisterClassW(&wc)) {
    log_error("Failed to create window class!");
    return false;
  }

  RECT window_rect;
  window_rect.left = (LONG)os.window_pos.x;
  window_rect.top = (LONG)os.window_pos.y;
  window_rect.right = (LONG)os.window_size.x + window_rect.left;
  window_rect.bottom = (LONG)os.window_size.y + window_rect.top;
  AdjustWindowRectEx(&window_rect, window_style_chosen, false, 0);

  w32(os).window = CreateWindowW(wc.lpszClassName, wc.lpszClassName, window_style_chosen, window_rect.left, window_rect.top, window_rect.right - window_rect.left,window_rect.bottom - window_rect.top, NULL, NULL, w32(os).instance, NULL);
  if (w32(os).window == 0) {
    log_error("Failed to create window!");
    return false;
  }

  ShowWindow(w32(os).window, w32(os).show_cmd);
  UpdateWindow(w32(os).window);
  w32(os).window_dc = GetDC(w32(os).window);

  w32(os).main_fiber = ConvertThreadToFiber(0);
  assert_msg(w32(os).main_fiber, "Failed to create main fiber");
  w32(os).msg_fiber = CreateFiber(0, _os_fiber_event_proc, 0);
  assert_msg(w32(os).msg_fiber, "Failed to create message fiber");

  if (os.cursor_visible == false)
    os_show_cursor(false);

  switch (os.render_backend) {
    case RenderBackend_Software: {
      os_init_software_render();
    } break;
    case RenderBackend_OpenGL1: {
      os_init_opengl();
    } break;
    default: assert_msg(0, "Invalid value for render backend");
    break;
  }

  os_pull_state();
  os.initialized = true;
  return true;
}

api bool os_game_loop() {
  assert_msg(os.initialized, "Platform is not initialized! Please call os_init");
  switch (os.render_backend) {
    case RenderBackend_Software: {
      if (os.screen) { // @Note: Draw screen
        U32 *p = os.screen->pixels;
        for (int y = 0; y < os.screen->y; y++) {
          for (int x = 0; x < os.screen->x; x++) {
            *p = ((*p & 0xff000000)) | ((*p & 0x00ff0000) >> 16) | ((*p & 0x0000ff00)) |
            ((*p & 0x000000ff) << 16);
            p += 1;
          }
        }

        HDC hdc = w32(os).window_dc;
        SelectObject(w32(os).screen.dc, w32(os).screen.dib);
        BitBlt(hdc, 0, 0, (LONG)os.screen->size.x, (LONG)os.screen->size.y, w32(os).screen.dc, 0, 0, SRCCOPY);
      }
    } break;
    case RenderBackend_OpenGL1: {
      SwapBuffers(w32(os).window_dc);
    } break;
    default:
    assert_msg(0, "Please select a rendering backend!");
    break;
  }
  arena_clear(os.frame_arena);
  os.update_time = os_time() - os.frame_start_time;
  os.update_end_cycles = __rdtsc();
  F64 frame_time = os.update_time;
  F64 ms_per_frame = os.ms_per_frame;
  if (frame_time < ms_per_frame) {
    if (w32(os).good_scheduling) {
      // @Todo: I have no idea if letting over sleep is bad or not
      // Busy waiting is chugging cpu alot more, not sure what to do
      F64 time_to_sleep = (ms_per_frame - frame_time) * 1000;
      if (time_to_sleep > 0) {
        Sleep((DWORD)time_to_sleep);
      }
    }

    do {
      frame_time = os_time() - os.frame_start_time;
    } while (frame_time < ms_per_frame);
  }
  os.frame++;
  os.delta_time = frame_time;
  os.fps = 1 / os.delta_time;
  os.time += os.delta_time;
  os.frame_start_time = os_time();
  os.update_begin_cycles = __rdtsc();
  os_pull_state();
  return !os.quit;
}

int main(int argc, char **argv);
int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR cmd_line, int show_cmd) {
  thread_ctx_init();
  w32(os).instance = instance;
  w32(os).show_cmd = show_cmd;
  w32(os).cmd_line = cmd_line;

  Arena frame_arena = {};
  arena_init(&frame_arena, "Frame arena"_s);
  os.perm_arena = &pernament_arena;
  os.frame_arena = &frame_arena;

  os.dpi_scale = 1;
  os.text.str = w32(os).text_buff;
  os.window_title = "Have a good day!"_s;
  os.window_pos.x = 0;
  os.window_pos.y = 50;
  os.window_size.x = 1280;
  os.window_size.y = 720;
  os.ms_per_frame = 1.f / 60.f;
  os.cursor_visible = true;

  if(AttachConsole(-1)) {
    freopen("CONIN$", "r",stdin);
    freopen("CONOUT$", "w",stdout);
    freopen("CONOUT$", "w",stderr);
  }

  return main(__argc, __argv);
}

//-----------------------------------------------------------------------------
// Font API
//-----------------------------------------------------------------------------
function FontGlyph extract_glyph(Allocator *arena, Win32FontCtx *ctx, wchar_t glyph) {
  SIZE size;
  GetTextExtentPoint32W(ctx->bitmap.dc, &glyph, 1, &size);
  TextOutW(ctx->bitmap.dc, 0, 0, &glyph, 1);

  // @Note: Find bitmap edges
  int minx = 100000;
  int miny = 100000;
  int maxx = -100000;
  int maxy = -100000;
  for (int y = 499; y >= 500 - size.cy; y--) {
    for (int x = 0; x < size.cx; x++) {
      if (ctx->bitmap.bitmap.pixels[x + y * (int)ctx->bitmap.bitmap.size.x] != 0) {
        if (minx > x)
          minx = x;
        if (miny > y)
          miny = y;
        if (maxx < x)
          maxx = x;
        if (maxy < y)
          maxy = y;
      }
    }
  }

  assert(minx >= 0 && miny >= 0);
  int bwidth = maxx - minx + 1;
  int bheight = maxy - miny + 1;
  U32 *cropped =
  (U32 *)exp_alloc(arena, sizeof(U32) * (U32)(bwidth) * (U32)(bheight));
  for (int y = miny; y <= maxy; y++) {
    for (int x = minx; x <= maxx; x++) {
      U32 value = ctx->bitmap.bitmap.pixels[x + y * (int)ctx->bitmap.bitmap.size.x];
      U32 *dst = cropped + ((size_t)(x - minx) + (size_t)(y - miny) * bwidth);
#if 1 // Premultiplied alpha
      F32 alpha = (F32)((value & 0x000000ff) >> 0);
      F32 rgb  = (F32)0xff*(alpha/255.f);
      U8 val    = (U8)(rgb + 0.5f);
      *dst = (((U32)(alpha+0.5f) << 24) | val << 16 | val << 8 | val);
#else
      U8 grey = (value & 0x000000ff);
      *dst = (grey << 24 | 0xff << 16 | 0xff << 8 | 0xff);
#endif
    }
  }
  exp_alloc(arena, sizeof(U32) * (U32)bwidth);

  // @Note: Calculate char metrics
  int glyph_descent = (499 - size.cy) - miny;

  FontGlyph result;
  INT width;
  GetCharWidth32W(ctx->bitmap.dc, glyph, glyph, &width);
  result.xadvance = (F32)width;
  result.bitmap = bitmap(cropped, vec2i(bwidth, bheight),
                         vec2((F32)-minx, (F32)ctx->text_metric.tmDescent + (F32)glyph_descent));
  return result;
}

function Win32FontCtx begin_font_extraction(char *filename, char *font_name, S64 height) {
  assert_msg(height < 500, "Height of font over 500");
  Win32FontCtx ctx = {};
  if (filename) {
    int fonts_added = AddFontResourceExA(filename, FR_PRIVATE, 0);
    assert_msg(fonts_added != 0, "AddFontResourceEx added 0 fonts");
  }

  ctx.bitmap = win32_create_bitmap(vec2i(500, -500));
  ctx.font =
    CreateFontA((S32)height, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, font_name);
  assert_msg(ctx.font != NULL, "CreateFont returned a 0 pointer");
  SelectObject(ctx.bitmap.dc, ctx.bitmap.dib);
  SelectObject(ctx.bitmap.dc, ctx.font);
  GetTextMetrics(ctx.bitmap.dc, &ctx.text_metric);
  ctx.result.height = (F32)height;
  ctx.result.descent = (F32)ctx.text_metric.tmDescent;
  ctx.result.ascent = (F32)ctx.text_metric.tmAscent;
  ctx.result.line_advance = (F32)(height + ctx.text_metric.tmExternalLeading);
  SetBkColor(ctx.bitmap.dc, RGB(0, 0, 0));
  SetTextColor(ctx.bitmap.dc, RGB(255, 255, 255));
  return ctx;
}

function Font end_font_extraction(Win32FontCtx *ctx) {
  win32_destroy_bitmap(&ctx->bitmap);
  DeleteObject(ctx->font);
  return ctx->result;
}

api Font os_load_font(Allocator *arena, S32 height, const char *font_name, const char *filename) {
  FontGlyph *glyphs = exp_alloc_array(arena, FontGlyph, 96, AF_ZeroMemory);
  Win32FontCtx font_ctx = begin_font_extraction((char *)filename, (char *)font_name, height);
  for (U32 i = '!'; i <= '~'; i++) {
    glyphs[i - '!'] = extract_glyph(arena, &font_ctx, (wchar_t)i);
  }
  Font font = end_font_extraction(&font_ctx);
  font.glyphs = glyphs;
  font.glyphs_len = '~' - '!';
  return font;
}

#undef w32

//-----------------------------------------------------------------------------
// Audio
//-----------------------------------------------------------------------------
#include <objbase.h>
#include <audioclient.h>
#include <audiopolicy.h>
#include <mmdeviceapi.h>

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
const IID IID_IAudioClient = __uuidof(IAudioClient);
const IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);

// NOTE: typedefines for the functions which are goint to be loaded
typedef HRESULT CoCreateInstanceFunction(REFCLSID rclsid, LPUNKNOWN *pUnkOuter, DWORD dwClsContext,
                                         REFIID riid, LPVOID *ppv);
typedef HRESULT CoInitializeExFunction(LPVOID pvReserved, DWORD dwCoInit);

// NOTE: empty functions(stubs) which are used when library fails to load
HRESULT CoCreateInstanceStub(REFCLSID rclsid, LPUNKNOWN *pUnkOuter, DWORD dwClsContext, REFIID riid,
                             LPVOID *ppv) {
  unused(rclsid);
  unused(pUnkOuter);
  unused(dwClsContext);
  unused(riid);
  unused(ppv);
  return S_FALSE;
}

HRESULT CoInitializeExStub(LPVOID pvReserved, DWORD dwCoInit) {
  unused(pvReserved);
  unused(dwCoInit);
  return S_FALSE;
}

// NOTE: pointers to the functions from the dll
CoCreateInstanceFunction *CoCreateInstanceFunctionPointer = CoCreateInstanceStub;
CoInitializeExFunction *CoInitializeExFunctionPointer = CoInitializeExStub;

// NOTE: Number of REFERENCE_TIME units per second
// One unit is equal to 100 nano seconds
#define REF_TIMES_PER_SECOND 10000000
#define REF_TIMES_PER_MSECOND 10000
#define w32(a) (*(Audio_Win32 *)(a)->platform)

struct Audio_Win32 {
  IMMDevice *device;
  IAudioClient *audio_client;

  IMMDeviceEnumerator *device_enum;
  IAudioRenderClient *audio_render_client;
  IAudioCaptureClient *audio_capture_client;
};
static_assert(sizeof(Audio::platform) > sizeof(Audio_Win32),
              "Audio::platform is too small to hold Audio_Win32 struct");

// Load COM Library functions dynamically,
// this way sound is not necessary to run the game
function B32 win32_load_com() {
  B32 result = true;
  HMODULE ole32Library = LoadLibraryA("ole32.dll");
  if (ole32Library) {
    CoCreateInstanceFunctionPointer =
    (CoCreateInstanceFunction *)GetProcAddress(ole32Library, "CoCreateInstance");
    if (!CoCreateInstanceFunctionPointer) {
      CoCreateInstanceFunctionPointer = CoCreateInstanceStub;
      log_error("CoCreateInstance failed to load");
      result = false;
    }
    CoInitializeExFunctionPointer = (CoInitializeExFunction *)GetProcAddress(ole32Library, "CoInitializeEx");
    if (!CoInitializeExFunctionPointer) {
      CoInitializeExFunctionPointer = CoInitializeExStub;
      log_error("CoInitializeEx failed to load");
      result = false;
    }
  } else {
    CoCreateInstanceFunctionPointer = CoCreateInstanceStub;
    CoInitializeExFunctionPointer = CoInitializeExStub;
    log_error("Failed to load OLE32.dll");
    result = false;
  }
  return result;
}

api void os_clean_audio(Audio *audio) {
  if (w32(audio).audio_client)
    w32(audio).audio_client->Stop();
  if (w32(audio).device_enum)
    w32(audio).device_enum->Release();
  if (w32(audio).device)
    w32(audio).device->Release();
  if (w32(audio).audio_client)
    w32(audio).audio_client->Release();
  if (w32(audio).audio_render_client)
    w32(audio).audio_render_client->Release();
  audio->initialized = false;
}

function DWORD win32_audio_thread(void *parameter) {
  Audio *audio = (Audio *)parameter;
  SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
  HANDLE buffer_ready_event = CreateEvent(0, 0, 0, 0);
  if (!buffer_ready_event) {
    return -1;
  }
  if (FAILED(w32(audio).audio_client->SetEventHandle(buffer_ready_event))) {
    return -1;
  }
  U32 buffer_frame_count;
  if (FAILED(w32(audio).audio_client->GetBufferSize(&buffer_frame_count))) {
    return -1;
  }
  // U32 buffer_sample_count = buffer_frame_count * audio->number_of_channels;
  if (FAILED(w32(audio).audio_client->Start())) {
    w32(audio).audio_client->Stop();
    return -1;
  }
  for (;;) {
    if (WaitForSingleObject(buffer_ready_event, INFINITE) != WAIT_OBJECT_0) {
      w32(audio).audio_client->Stop();
      return -1;
    }
    U32 padding_frame_count;
    if (FAILED(w32(audio).audio_client->GetCurrentPadding(&padding_frame_count))) {
      w32(audio).audio_client->Stop();
      return -1;
    }
    U32 *samples;
    U32 fill_frame_count = buffer_frame_count - padding_frame_count;
    if (FAILED(w32(audio).audio_render_client->GetBuffer(fill_frame_count, (BYTE **)&samples))) {
      w32(audio).audio_client->Stop();
      return -1;
    }
    audio->callback(audio, samples, fill_frame_count);
    if (FAILED(w32(audio).audio_render_client->ReleaseBuffer(fill_frame_count, 0))) {
      w32(audio).audio_client->Stop();
      return -1;
    }
  }
  return 0;
}

function AUDIO_CALLBACK(default_audio_callback) {
  memory_zero(buffer, (U64)frames_to_fill * (U64)audio->samples_per_second);
}

api B32 os_init_audio(Audio *audio) {
  audio->bits_per_sample = 16;
  if (audio->number_of_channels == 0)
    audio->number_of_channels = 2;
  if (audio->samples_per_second == 0)
    audio->samples_per_second = 44100;
  if (audio->callback == 0)
    audio->callback = default_audio_callback;

  B32 success = win32_load_com();
  if (!success) {
    return false;
  }
  if (FAILED(CoInitializeExFunctionPointer(0, COINITBASE_MULTITHREADED))) {
    log_error("Failed to initialize COM, CoInitializeEx");
    return false;
  }
  if (FAILED(CoCreateInstanceFunctionPointer(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL,
                                             IID_IMMDeviceEnumerator, (LPVOID *)&w32(audio).device_enum))) {
    log_error("Failed to initialize COM, CoCreateInstance");
    return false;
  }
  if (FAILED(w32(audio).device_enum->GetDefaultAudioEndpoint(eRender, eConsole, &w32(audio).device))) {
    os_clean_audio(audio);
    log_error("Failed to initialize WASAPI, GetDefaultAudioEndpoint");
    return false;
  }
  if (FAILED(
             w32(audio).device->Activate(IID_IAudioClient, CLSCTX_ALL, 0, (void **)&w32(audio).audio_client))) {
    os_clean_audio(audio);
    log_error("Failed to initialize WASAPI, "
              "w32(audio).device->Activate(IID_IAudioClient,");
    return false;
  }

  WAVEFORMATEX waveFormat = {};
  waveFormat.wFormatTag = WAVE_FORMAT_PCM;
  waveFormat.nChannels = audio->number_of_channels;
  waveFormat.nSamplesPerSec = audio->samples_per_second;
  waveFormat.wBitsPerSample = audio->bits_per_sample;
  waveFormat.nBlockAlign = waveFormat.nChannels * waveFormat.wBitsPerSample / 8;
  waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
  REFERENCE_TIME requestedBufferDuration = REF_TIMES_PER_MSECOND * 40;
  if (FAILED(w32(audio).audio_client->Initialize(AUDCLNT_SHAREMODE_SHARED,
                                                 AUDCLNT_STREAMFLAGS_EVENTCALLBACK | AUDCLNT_STREAMFLAGS_RATEADJUST |
                                                 AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM | AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY,
                                                 requestedBufferDuration, 0, &waveFormat, 0))) {
    os_clean_audio(audio);
    log_error("Failed to initialize WASAPI, w32(audio).audio_client->Initialize");
    return false;
  }
  if (FAILED(w32(audio).audio_client->GetService(IID_IAudioRenderClient,
                                                 (void **)&w32(audio).audio_render_client))) {
    os_clean_audio(audio);
    log_error("Failed to initialize WASAPI, "
              "w32(audio).audio_client->GetService(IID_IAudioRenderClient");
    return false;
  }
  if (FAILED(w32(audio).audio_client->GetBufferSize(&audio->buffer_frame_count))) {
    os_clean_audio(audio);
    log_error("Failed to initialize WASAPI, w32(audio).audio_client->GetBufferSize");
    return false;
  }
  HANDLE thread_handle = CreateThread(0, 0, win32_audio_thread, audio, 0, 0);
  if (!thread_handle) {
    os_clean_audio(audio);
    log_error("Failed to initialize WASAPI, CreateThread returned 0 in handle");
    return false;
  }
  if (thread_handle == INVALID_HANDLE_VALUE) {
    os_clean_audio(audio);
    log_error("Failed to initialize WASAPI, CreateThread for "
              "audio failed with INVALID HANDLE VALUE");
    return false;
  }
  CloseHandle(thread_handle);
  audio->initialized = true;
#undef w32
  return true;
}

