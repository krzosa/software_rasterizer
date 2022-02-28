enum UIWidgetKind {
  UIWidgetKind_None,
  UIWidgetKind_Boolean,
  UIWidgetKind_Image,
  UIWidgetKind_Label,
  UIWidgetKind_Group,
};

struct UISetup {
  UIWidgetKind kind;
  S8 text;
  union {
    void *v;
    Bitmap *image;
    B32    *b32;
    S8     *label;
  };
};
#define UI_BOOL(text, x) {UIWidgetKind_Boolean,text,(void*)(x)}
#define UI_IMAGE(x)      {UIWidgetKind_Image,string_null,(void*)(x)}
#define UI_LABEL(x)      {UIWidgetKind_Label,string_null,(void*)(x)}

struct UIWidget {
  UIWidgetKind kind;
  UIWidget    *next;
  UIWidget    *prev;
  UIWidget    *first_child;
  UIWidget    *last_child;
 
  S8 text;
  Vec2 size;
  union {
    Bitmap *image; 
    B32    *b32; 
    S8     *label;
  } ptr;
};

struct UI : UIWidget {
  Arena arena;

  UIWidget *hot;
  UIWidget *active;
};

FUNCTION UIWidget *ui_new_widget(Arena *arena, UIWidgetKind kind) {
  UIWidget *result = PUSH_STRUCT(arena, UIWidget);
  result->kind = kind;
  return result;
}

FUNCTION void ui_push_child(UIWidget *widget, UIWidget *child) {
  DLL_QUEUE_PUSH(widget->first_child, widget->last_child, child);
}

FUNCTION UIWidget *ui_push_child(Arena *arena, UIWidget *widget, UIWidgetKind kind) {
  UIWidget *result = ui_new_widget(arena, kind);
  ui_push_child(widget, result);
  return result;
}

FUNCTION UIWidget *ui_push_image(Arena *arena, UIWidget *widget, Bitmap *img) {
  UIWidget *result = ui_push_child(arena, widget, UIWidgetKind_Image);
  result->ptr.image = img;

  F32 ratio = (F32)result->ptr.image->x / (F32)result->ptr.image->y;
  result->size.y = 64;
  result->size.x = 64 * ratio;
  return result;
}

FUNCTION UIWidget *ui_push_bool(Arena *arena, UIWidget *widget, B32 *b32) {
  UIWidget *result = ui_push_child(arena, widget, UIWidgetKind_Boolean);
  result->ptr.b32 = b32;
  return result;
}

FUNCTION UIWidget *ui_push_string(Arena *arena, UIWidget *widget, S8 *string) {
  UIWidget *result = ui_push_child(arena, widget, UIWidgetKind_Label);
  result->ptr.label = string;
  return result;
}

FUNCTION UI ui_make(Arena *arena, UISetup *setup, U64 len) {
  UI result = {};
  result.arena = arena_sub(arena, MiB(16));
  UIWidget *parent = &result;
  for (UISetup *s = setup; s != (setup+len); s++) {
    switch (s->kind) {
      case UIWidgetKind_Image: {
        ui_push_image(&result.arena, parent, s->image);
      } break;
      case UIWidgetKind_Boolean: {
        ui_push_bool(&result.arena, parent, s->b32)->text = s->text;
      } break;
      case UIWidgetKind_Label: {
        ui_push_string(&result.arena, parent, s->label);
      } break;
      INVALID_DEFAULT_CASE;
    }
  }
  return result;
}

FUNCTION B32 ui_mouse_test(UI *ui, UIWidget *w, Vec4 rect) {
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

FUNCTION void ui_end_frame(Bitmap *dst, UI *ui, Font *font) {
  Scratch scratch;
  Vec2 pos = vec2(0, (F32)dst->y);
  for (UIWidget *w = ui->first_child; w; w = w->next) {
    Vec4 rect = {};
    switch (w->kind) {
      case UIWidgetKind_Image: {
        pos.y -= w->size.y;
        rect = vec4(pos, w->size);
        ui_mouse_test(ui, w, rect);
        S8 string = string_format(scratch, "%d %d", w->ptr.image->x, w->ptr.image->y);
        r_draw_string(dst, font, string, pos);
        r_draw_bitmap(dst, w->ptr.image, pos, w->size);
        if (ui->active == w) {
          F32 ratio = (F32)w->ptr.image->y / (F32)w->ptr.image->x;
          w->size.x -= os.delta_mouse_pos.x;
          w->size.y = w->size.x * ratio;
        }
        if (ui->hot == w) {
          r_draw_rect(dst, rect.x, rect.y, rect.width, rect.height, vec4(1, 1, 1, 0.1f));
        }
      } break;
      case UIWidgetKind_Boolean: {
        pos.y -= font->height;
        Vec4 color = vec4(0, 0, 0, 1);
        S8 string = string_format(scratch, "%s %d", w->text, *w->ptr.b32);
        rect = r_get_string_rect(font, string, pos);
        B32 clicked = ui_mouse_test(ui, w, rect);
        if (clicked) *w->ptr.b32 = !*w->ptr.b32;
        if (ui->hot == w) {
          color = vec4(0.4f, 0.4f, 0.4f, 1.f);
        }
        rect.y = rect.y-font->line_advance / 5;
        r_draw_rect(dst, rect.x, rect.y, rect.width, rect.height, color);
        rect = r_draw_string(dst, font, string, pos);
        pos.y -= rect.height - font->height;
      } break;
      case UIWidgetKind_Label: {
        pos.y -= font->height;
        rect = r_draw_string(dst, font, *w->ptr.label, pos);
        pos.y -= rect.height - font->height;
      } break;
      INVALID_DEFAULT_CASE;
    }
  }
}