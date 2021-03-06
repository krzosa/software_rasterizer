enum UIWidgetKind {
  UIWidgetKind_None,
  UIWidgetKind_Boolean,
  UIWidgetKind_Image,
  UIWidgetKind_Label,
  UIWidgetKind_Option,
  UIWidgetKind_Signal,
  UIWidgetKind_Group,
};

#define UI_SIGNAL_CALLBACK(name) void name()
typedef UI_SIGNAL_CALLBACK(UISignalCallback);
struct UISetup {
  UIWidgetKind kind;
  String text;
  union {
    void *v;
    Bitmap *image;
    B32    *b32;
    String     *label;
    S32    *option;
    UISignalCallback *signal_callback;
  };
  S32 option_max;
};
#define UI_BOOL(text, x) {UIWidgetKind_Boolean,text,(void*)(x)}
#define UI_IMAGE(x)      {UIWidgetKind_Image,{},(void*)(x)}
#define UI_LABEL(x)      {UIWidgetKind_Label,{},(void*)(x)}
#define UI_OPTION(text,x,option_max){UIWidgetKind_Option,text,(void*)(x),(option_max)}
#define UI_SIGNAL(text,x){UIWidgetKind_Signal,text,(void*)(x)}

struct UIWidget {
  UIWidgetKind kind;
  UIWidget    *next;
  UIWidget    *prev;
  UIWidget    *first_child;
  UIWidget    *last_child;

  String text;
  Vec2 size;
  S32 option_max;
  union {
    Bitmap *image;
    B32    *b32;
    String     *label;
    S32    *option;
    UISignalCallback *signal_callback;
  } ptr;
};

struct UI : UIWidget {
  Arena arena;

  UIWidget *hot;
  UIWidget *active;
};

function UIWidget *ui_new_widget(Allocator *arena, UIWidgetKind kind) {
  UIWidget *result = exp_alloc_type(arena, UIWidget, AF_ZeroMemory);
  result->kind = kind;
  return result;
}

function void ui_push_child(UIWidget *widget, UIWidget *child) {
  DLLQueueAdd(widget->first_child, widget->last_child, child);
}

function UIWidget *ui_push_child(Arena *arena, UIWidget *widget, UIWidgetKind kind) {
  UIWidget *result = ui_new_widget(arena, kind);
  ui_push_child(widget, result);
  return result;
}

function UIWidget *ui_push_image(Arena *arena, UIWidget *widget, Bitmap *img) {
  UIWidget *result = ui_push_child(arena, widget, UIWidgetKind_Image);
  result->ptr.image = img;

  F32 ratio = (F32)result->ptr.image->x / (F32)result->ptr.image->y;
  result->size.y = 64;
  result->size.x = 64 * ratio;
  return result;
}

function UIWidget *ui_push_bool(Arena *arena, UIWidget *widget, String string, B32 *b32) {
  UIWidget *result = ui_push_child(arena, widget, UIWidgetKind_Boolean);
  result->text = string;
  result->ptr.b32 = b32;
  return result;
}

function UIWidget *ui_push_string(Arena *arena, UIWidget *widget, String *string) {
  UIWidget *result = ui_push_child(arena, widget, UIWidgetKind_Label);
  result->ptr.label = string;
  return result;
}

function UIWidget *ui_push_option(Arena *arena, UIWidget *widget, String string, S32 *option, S32 option_max) {
  UIWidget *result = ui_push_child(arena, widget, UIWidgetKind_Option);
  result->text = string;
  result->ptr.option = option;
  result->option_max = option_max;
  return result;
}

function UIWidget *ui_push_signal(Arena *arena, UIWidget *widget, String string, UISignalCallback *callback) {
  UIWidget *result = ui_push_child(arena, widget, UIWidgetKind_Signal);
  result->text = string;
  result->ptr.signal_callback = callback;
  return result;
}

function UI ui_make(UISetup *setup, U64 len) {
  UI result = {};
  arena_init(&result.arena, "UI_Arena"_s);
  UIWidget *parent = &result;
  for (UISetup *s = setup; s != (setup+len); s++) {
    switch (s->kind) {
      case UIWidgetKind_Image: {
        ui_push_image(&result.arena, parent, s->image);
      } break;
      case UIWidgetKind_Boolean: {
        ui_push_bool(&result.arena, parent, s->text, s->b32);
      } break;
      case UIWidgetKind_Label: {
        ui_push_string(&result.arena, parent, s->label);
      } break;
      case UIWidgetKind_Option: {
        ui_push_option(&result.arena, parent, s->text, s->option, s->option_max);
      } break;
      case UIWidgetKind_Signal: {
        ui_push_signal(&result.arena, parent, s->text, s->signal_callback);
      } break;
      invalid_default_case;
    }
  }
  return result;
}

function B32 ui_mouse_test(UI *ui, UIWidget *w, Vec4 rect) {
  B32 result = false;
  if (os.mouse_pos.x > rect.x && os.mouse_pos.x < rect.x + rect.width &&
      os.mouse_pos.y > rect.y && os.mouse_pos.y < rect.y + rect.height) {
    ui->hot = w;
    if (os.key[Key_MouseLeft].down) {
      ui->active = w;
    }
  }
  else if (w == ui->hot) {
    ui->hot = 0;
  }

  if (os.key[Key_MouseLeft].released) {
    if (ui->active == w) {
      if (ui->hot == w)
        result = true;
      ui->active = 0;
    }
  }

  return result;
}

function void ui_end_frame(Bitmap *dst, UI *ui, Font *font) {
  Scratch scratch;
  Vec2 pos = vec2(0, (F32)dst->y);
  for (UIWidget *w = ui->first_child; w; w = w->next) {
    Vec4 rect = {};
    switch (w->kind) {
      case UIWidgetKind_Image: {
        pos.y -= w->size.y;
        rect = vec4(pos, w->size);
        ui_mouse_test(ui, w, rect);
        String string = string_fmt(scratch, "%d %d", w->ptr.image->x, w->ptr.image->y);
        draw_string(dst, font, string, pos);
        draw_bitmap(dst, w->ptr.image, pos, w->size);
        if (ui->active == w) {
          F32 ratio = (F32)w->ptr.image->y / (F32)w->ptr.image->x;
          w->size.x -= os.delta_mouse_pos.x;
          w->size.y = w->size.x * ratio;
        }
        if (ui->hot == w) {
          draw_rect(dst, rect.x, rect.y, rect.width, rect.height, vec4(1, 1, 1, 0.1f));
        }
      } break;
      case UIWidgetKind_Boolean: {
        pos.y -= font->height;
        Vec4 color = vec4(0, 0, 0, 1);
        String string = string_fmt(scratch, "%s %d", w->text, *w->ptr.b32);
        rect = get_string_rect(font, string, pos);
        B32 clicked = ui_mouse_test(ui, w, rect);
        if (clicked) *w->ptr.b32 = !*w->ptr.b32;
        if (ui->hot == w) {
          color = vec4(0.4f, 0.4f, 0.4f, 1.f);
        }
        rect.y = rect.y-font->line_advance / 5;
        draw_rect(dst, rect.x, rect.y, rect.width, rect.height, color);
        rect = draw_string(dst, font, string, pos);
        pos.y -= rect.height - font->height;
      } break;
      case UIWidgetKind_Label: {
        pos.y -= font->height;
        rect = draw_string(dst, font, *w->ptr.label, pos);
        pos.y -= rect.height - font->height;
      } break;
      case UIWidgetKind_Option: {
        pos.y -= font->height;
        Vec4 color = vec4(0, 0, 0, 1);
        String string = string_fmt(scratch, "%Q %Q", w->text, *w->ptr.option);
        rect = get_string_rect(font, string, pos);
        B32 clicked = ui_mouse_test(ui, w, rect);
        if (clicked) {
          *w->ptr.b32 = (*w->ptr.b32+1) % w->option_max;
        }
        if (ui->hot == w) {
          color = vec4(0.4f, 0.4f, 0.4f, 1.f);
        }
        rect.y = rect.y-font->line_advance / 5;
        draw_rect(dst, rect.x, rect.y, rect.width, rect.height, color);
        rect = draw_string(dst, font, string, pos);
        pos.y -= rect.height - font->height;
      } break;
      case UIWidgetKind_Signal: {
        pos.y -= font->height;
        Vec4 color = vec4(0, 0, 0, 1);
        String string = string_fmt(scratch, "%Q", w->text);
        rect = get_string_rect(font, string, pos);
        B32 clicked = ui_mouse_test(ui, w, rect);
        if (clicked) {
          w->ptr.signal_callback();
        }
        if (ui->hot == w) {
          color = vec4(0.4f, 0.4f, 0.4f, 1.f);
        }
        rect.y = rect.y-font->line_advance / 5;
        draw_rect(dst, rect.x, rect.y, rect.width, rect.height, color);
        rect = draw_string(dst, font, string, pos);
        pos.y -= rect.height - font->height;
      } break;
      invalid_default_case;
    }
  }
}