/* 
* The OS layer should expect ABGR writes to the screen, the bitmap is bottom up(origin 0,0 is in left bottom corner)
*/

#define _CRT_SECURE_NO_WARNINGS
#include "platform.h"
#include <windows.h>
#include <shellscalingapi.h>


typedef HRESULT tSetProcessDpiAwareness(PROCESS_DPI_AWARENESS);

Image screen;
float* depth_buffer;
bool keydown_a;
bool keydown_b;

GLOBAL bool g_app_is_running = true;
GLOBAL HBITMAP g_screen_dib;
GLOBAL HDC g_screen_dc;
GLOBAL HDC g_window_dc;
GLOBAL HWND g_hwnd;
GLOBAL HINSTANCE g_hinstance;
GLOBAL int g_cmdshow;


FUNCTION
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  LRESULT result = 0;
  switch (uMsg) {
  case WM_CLOSE: DestroyWindow(hwnd); g_app_is_running = false; break;
  case WM_DESTROY: PostQuitMessage(0); g_app_is_running = false; break;
  case WM_SYSKEYDOWN:
  case WM_KEYDOWN: {
    switch (wParam) {
    case VK_ESCAPE: g_app_is_running = false; break;
    case 0x4F: keydown_a = true; break;
    case 0x50: keydown_b = true; break;
    }
  } break;
  case WM_SYSKEYUP:
  case WM_KEYUP: {
    switch (wParam) {
    case 0x4F: keydown_a = false; break;
    case 0x50: keydown_b = false; break;
    }
  } break;
  default: result = DefWindowProc(hwnd, uMsg, wParam, lParam);
  }
  return result;
}

FUNCTION
void Win32_ScreenInit(int window_x, int window_y) {
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
  g_screen_dib = CreateDIBSection(g_window_dc, &bminfo, DIB_RGB_COLORS, (void**)&mem, 0, 0);
  g_screen_dc = CreateCompatibleDC(g_window_dc);
  screen.pixels = (U32*)mem;
  depth_buffer = (float*)malloc((size_t)window_x * (size_t)window_y * sizeof(float));
  screen.x = window_x;
  screen.y = window_y;
}

void OS_Init(OSInitArgs i) {
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
  wc.hInstance = g_hinstance;
  wc.lpszClassName = CLASS_NAME;
  ASSERT(RegisterClass(&wc));

  g_hwnd = CreateWindowEx(
    0,                              // Optional window styles.
    CLASS_NAME,                     // Window class
    L"Have a wonderful day!",    // Window text
    WS_OVERLAPPEDWINDOW,            // Window style
    CW_USEDEFAULT, CW_USEDEFAULT, i.window_x, i.window_y, // Size and position
    NULL,       // Parent window    
    NULL,       // Menu
    g_hinstance,  // Instance handle
    NULL        // Additional application data
  );
  if (g_hwnd == 0) {
    ASSERT(!"Failed to create window");
    return;
  }
  

  ShowWindow(g_hwnd, SW_SHOW);
  RECT rect;
  GetWindowRect(g_hwnd, &rect);
  g_window_dc = GetWindowDC(g_hwnd);
  Win32_ScreenInit(rect.right - rect.left, rect.bottom - rect.top);
}

bool OS_GameLoop() {
  MSG msg = { };
  while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE) > 0) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  // @Note: Free the screen on window resize
  RECT rect;
  GetWindowRect(g_hwnd, &rect);
  int new_width = rect.right - rect.left;
  int new_height = rect.bottom - rect.top;
  if (new_width != screen.x || new_height != screen.y) {
    screen.x = new_width;
    screen.y = new_height;
    if (screen.pixels) {
      screen.pixels = 0;
      free(depth_buffer);
      DeleteDC(g_screen_dc);
      DeleteObject(g_screen_dib);
    }
  }

  // @Note: Create drawable screen
  if (!screen.pixels) {
    Win32_ScreenInit((LONG)screen.x, (LONG)screen.y);
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

    SelectObject(g_screen_dc, g_screen_dib);
    BitBlt(g_window_dc, 0, 0, (int)screen.x, (int)screen.y, g_screen_dc, 0, 0, SRCCOPY);
  }
  
  Sleep(16);
  return g_app_is_running;
}

int main();
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
  g_hinstance = hInstance;
  g_cmdshow = nCmdShow;
  return main();
}

#include <stdio.h>
void OS_Message(const char* format, ...) {
  char buffer[1024];
  va_list args;
  va_start(args, format);
  vsnprintf(buffer, 1024, format, args);
  va_end(args);

  MessageBoxA(0, buffer, "Error!", 0);
}

char* OS_ReadFile(const char* path) {
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