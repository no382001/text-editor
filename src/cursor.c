
#include "cursor.h"
#include "raylib.h"
#include "utils.h"

Cursor cursor = {0, 0}; // top left

// dont go over the column
static void cursor_col_bound_check(Document *d) {
  LineNode *line = document_find_line(d, cursor.line);
  chk_ptr(line);
  if (cursor.column > line->head->size) {
    cursor.column = line->head->size;
  }
}

static void key_up(Document *d) {
  if (cursor.line > 0) {
    cursor.line--;
    cursor_col_bound_check(d);
  }
}

static void key_down(Document *d) {
  if (cursor.line < d->line_count - 1) {
    cursor.line++;
    cursor_col_bound_check(d);
  }
}

static void key_left(Document *d) {
  if (cursor.column > 0) {
    // normal
    cursor.column--;
  } else if (cursor.line > 0) {
    // go up a line to the end of the prev col
    cursor.line--;
    LineNode *line = document_find_line(d, cursor.line);
    chk_ptr(line);
    cursor.column = line->head->size;
  }
}

static void key_right(Document *d) {
  LineNode *line = document_find_line(d, cursor.line);
  chk_ptr(line);
  if (cursor.column < line->head->size) {
    // normal
    cursor.column++;
  } else if (cursor.line < d->line_count - 1) {
    // go over to the next line first colum
    cursor.line++;
    cursor.column = 0;
  }
}

static void handle_single_press(Document *d) {
  if (IsKeyPressed(KEY_UP)) {
    key_up(d);
  }
  if (IsKeyPressed(KEY_DOWN)) {
    key_down(d);
  }
  if (IsKeyPressed(KEY_LEFT)) {
    key_left(d);
  }
  if (IsKeyPressed(KEY_RIGHT)) {
    key_right(d);
  }
}

extern int scroll_offset;
extern int font_size;
extern int line_padding;

void handle_keys(Document *d) {

  { // single keypress
    handle_single_press(d);
  }

  static int framesHeld = 0;
  const int holdThreshold = 5;
  { // continous holdiong

    if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_LEFT) ||
        IsKeyDown(KEY_RIGHT)) {
      framesHeld++;
      if (framesHeld >= holdThreshold) {
        if (IsKeyDown(KEY_UP)) {
          key_up(d);
        }
        if (IsKeyDown(KEY_DOWN)) {
          key_down(d);
        }
        if (IsKeyDown(KEY_LEFT)) {
          key_left(d);
        }
        if (IsKeyDown(KEY_RIGHT)) {
          key_right(d);
        }
      }
    } else {
      framesHeld = 0;
    }
  }

  int visible_lines = (GetScreenHeight() - 200) / (font_size + line_padding);

  // if the cursor's line is before the first visible line adjust the scroll
  // offset
  if (cursor.line < scroll_offset / (font_size + line_padding)) {
    scroll_offset = cursor.line * (font_size + line_padding);
  }

  // if the cursor's line is after the last visible line adjust the scroll
  // offset
  if (cursor.line >=
      (scroll_offset / (font_size + line_padding)) + visible_lines) {
    scroll_offset =
        (cursor.line - visible_lines + 1) * (font_size + line_padding);
  }

  int key = GetKeyPressed();
  const char *buff = GetKeyString(key);
  if (key > 0){
    log_message(DEBUG, "keypress: %s, crsor:%d,%d\n", buff,cursor.line,cursor.column);
  }

  key = GetCharPressed();
  if (key > 0) {
    LineNode *line = document_find_line(d, cursor.line);
    chk_ptr(line);
    line_node_insert(line, cursor.column+1, key);
    key_right(d);
  }

  if (IsKeyPressed(KEY_BACKSPACE)) {
    if (cursor.line >= 0 && cursor.column >= 0) {
      LineNode *line = document_find_line(d, cursor.line);
      chk_ptr(line);
      line_node_delete(line, cursor.column-1, 1);
      key_left(d);
    }   
  }
}