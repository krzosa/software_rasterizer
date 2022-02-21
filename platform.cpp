/* 
* The OS layer should expect ABGR writes to the screen, the bitmap is bottom up(origin 0,0 is in left bottom corner)
*/
#if 0

#define _CRT_SECURE_NO_WARNINGS
#include "platform.h"
#include <windows.h>
#include <shellscalingapi.h>

struct OSWin32 {
  HBITMAP screen_dib;
  HDC screen_dc;
  HDC window_dc;
  HWND hwnd;
  HINSTANCE hinstance;
};

typedef HRESULT tSetProcessDpiAwareness(PROCESS_DPI_AWARENESS);
GLOBAL OS *wnd_proc_os = 0;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  LRESULT result = 0;
  switch (uMsg) {
    case WM_CLOSE: DestroyWindow(hwnd); wnd_proc_os->app_is_running = false; break;
    case WM_DESTROY: PostQuitMessage(0); wnd_proc_os->app_is_running = false; break;
  case WM_SYSKEYDOWN:
  case WM_KEYDOWN: {
    switch (wParam) {
      case VK_ESCAPE: wnd_proc_os->app_is_running = false; break;
      case 0x4F: wnd_proc_os->keydown_a = true; break;
      case 0x50: wnd_proc_os->keydown_b = true; break;
      case VK_F1: wnd_proc_os->keydown_f1 = true; break;
      case VK_F2: wnd_proc_os->keydown_f2 = true; break;
      case VK_F3: wnd_proc_os->keydown_f3 = true; break;
    }
  } break;
  case WM_SYSKEYUP:
  case WM_KEYUP: {
    switch (wParam) {
      case 0x4F: wnd_proc_os->keydown_a = false; break;
      case 0x50: wnd_proc_os->keydown_b = false; break;
      case VK_F1: wnd_proc_os->keydown_f1 = false; break;
      case VK_F2: wnd_proc_os->keydown_f2 = false; break;
      case VK_F3: wnd_proc_os->keydown_f3 = false; break;
    }
  } break;
  default: result = DefWindowProc(hwnd, uMsg, wParam, lParam);
  }
  return result;
}

FUNCTION
void win32_screen_init(OS *os, int window_x, int window_y) {
  OSWin32 *w32 = (OSWin32 *)os->os_internal_data;
  BITMAPINFO bminfo = {};
  bminfo.bmiHeader.biSize = sizeof(bminfo.bmiHeader);
  bminfo.bmiHeader.biWidth = window_x;
  bminfo.bmiHeader.biHeight = window_y;
  bminfo.bmiHeader.biPlanes = 1;
  bminfo.bmiHeader.biBitCount = 32;
  bminfo.bmiHeader.biCompression = BI_RGB; // AA RR GG BB
  bminfo.bmiHeader.biXPelsPerMeter = 1;
  bminfo.bmiHeader.biYPelsPerMeter = 1;

  void* mem = 0;
  w32->screen_dib = CreateDIBSection(w32->window_dc, &bminfo, DIB_RGB_COLORS, (void**)&mem, 0, 0);
  w32->screen_dc = CreateCompatibleDC(w32->window_dc);
  os->screen.pixels = (U32*)mem;
  os->screen.x = window_x;
  os->screen.y = window_y;
}

void OS::init(OSInitArgs i) {
  OSWin32 *w32 = (OSWin32 *)os_internal_data;
  w32->hinstance = (HINSTANCE)GetModuleHandle(NULL);
  HMODULE shcore = LoadLibraryA("Shcore.dll");
  if (shcore) {
    tSetProcessDpiAwareness* set_dpi_awr = (tSetProcessDpiAwareness*)GetProcAddress(shcore, "SetProcessDpiAwareness");
    if (set_dpi_awr) {
      HRESULT hr = set_dpi_awr(PROCESS_PER_MONITOR_DPI_AWARE);
      ASSERT(SUCCEEDED(hr));
    }
  }

  const wchar_t CLASS_NAME[] = L"Hello!";
  WNDCLASS wc = { };
  wc.lpfnWndProc = WindowProc;
  wc.hInstance = w32->hinstance;
  wc.lpszClassName = CLASS_NAME;
  ASSERT(RegisterClass(&wc));

  w32->hwnd = CreateWindowEx(
    0,                              // Optional window styles.
    CLASS_NAME,                     // Window class
    L"Have a wonderful day!",    // Window text
    WS_OVERLAPPEDWINDOW,            // Window style
    CW_USEDEFAULT, CW_USEDEFAULT, i.window_x, i.window_y, // Size and position
    NULL,       // Parent window    
    NULL,       // Menu
    w32->hinstance,  // Instance handle
    NULL        // Additional application data
  );
  if (w32->hwnd == 0) {
    ASSERT(!"Failed to create window");
    return;
  }
  

  ShowWindow(w32->hwnd, SW_SHOW);
  RECT rect;
  GetWindowRect(w32->hwnd, &rect);
  w32->window_dc = GetWindowDC(w32->hwnd);
  win32_screen_init(this, rect.right - rect.left, rect.bottom - rect.top);
  app_is_running = true;
}

bool OS::game_loop() {
  OSWin32 *w32 = (OSWin32 *)os_internal_data;

  MSG msg = { };
  wnd_proc_os = this;
  while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE) > 0) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  // @Note: Free the screen on window resize
  RECT rect;
  GetWindowRect(w32->hwnd, &rect);
  int new_width = rect.right - rect.left;
  int new_height = rect.bottom - rect.top;
  if (new_width != screen.x || new_height != screen.y) {
    screen.x = new_width;
    screen.y = new_height;
    if (screen.pixels) {
      screen.pixels = 0;
      DeleteDC(w32->screen_dc);
      DeleteObject(w32->screen_dib);
    }
  }

  // @Note: Create drawable screen
  if (!screen.pixels) {
    win32_screen_init(this, (LONG)screen.x, (LONG)screen.y);
  }

  // @Note: Draw screen to window
  U32* p = screen.pixels;
  if (p != 0) {
    // @Note: Convert 0xABGR to 0xARGB
    for (int y = 0; y < screen.y; y++) {
      for (int x = 0; x < screen.x; x++) {
        *p = ((*p & 0xff000000)) | ((*p & 0x00ff0000) >> 16) | ((*p & 0x0000ff00)) | ((*p & 0x000000ff) << 16);
        p += 1;
      }
    }

    SelectObject(w32->screen_dc, w32->screen_dib);
    BitBlt(w32->window_dc, 0, 0, (int)screen.x, (int)screen.y, w32->screen_dc, 0, 0, SRCCOPY);
  }
  
  Sleep(16);
  return app_is_running;
}

int main();
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
  return main();
}

#include <stdio.h>
void OS::message(int debug, const char* format, ...) {
  char buffer[1024];
  va_list args;
  va_start(args, format);
  vsnprintf(buffer, 1024, format, args);
  va_end(args);
  if (debug) {
    OutputDebugStringA(buffer);
  }
  else {
    MessageBoxA(0, buffer, "Error!", 0);
  }
}

char* OS::read_file(const char* path) { 
  char* result = 0;
  FILE* f = fopen(path, "rb");
  if (f) {
    fseek(f, 0, SEEK_END);
    U64 fsize = ftell(f);
    fseek(f, 0, SEEK_SET); 

    result = (char*)malloc(fsize + 1ll);
    if (result) {
      fread(result, fsize, 1, f);
      result[fsize] = 0;
    }
    else {
      ASSERT(!"Malloc failed!")
    }
    fclose(f);
  }
  else {
    ASSERT(!"Failed to read file");
  }
  
  return result;
}

#endif