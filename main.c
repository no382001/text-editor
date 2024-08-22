#include <raylib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "./editor.h"

#define FONT_WIDTH 128
#define FONT_HEIGHT 64
#define FONT_COLS 18
#define FONT_ROWS 7
#define FONT_CHAR_WIDTH (FONT_WIDTH / FONT_COLS)
#define FONT_CHAR_HEIGHT (FONT_HEIGHT / FONT_ROWS)
#define FONT_SCALE 5

#define ASCII_DISPLAY_LOW 32
#define ASCII_DISPLAY_HIGH 126

Editor editor = {};

void render_cursor() {
  Vector2 pos =
      (Vector2){.x = (float)editor.cursor_col * 12,
                .y = (float)editor.cursor_row * 12};

  Rectangle cursor_rect = {
      .x = pos.x,
      .y = pos.y,
      .width = 12,
      .height = 12,
  };

  DrawRectangleRec(cursor_rect, WHITE);
}

int main(void) {
  InitWindow(800, 600, "Text Editor");
  SetTargetFPS(60);

  editor_insert_text_before_cursor(&editor, "dhjfhskjdfhkjshdf");
  editor_insert_new_line(&editor);
  editor_insert_text_before_cursor(&editor, "3j4k23l4j");
  editor_insert_new_line(&editor);
  editor_insert_text_before_cursor(&editor, "456kj356klj35l6j");
  editor_insert_new_line(&editor);
  editor_insert_text_before_cursor(&editor, "46jkl45jkljclslkj");
  editor_insert_new_line(&editor);
  editor_insert_text_before_cursor(&editor, "tjk5kfkdjgk");

  while (!WindowShouldClose()) {

    if (IsKeyPressed(KEY_BACKSPACE)) {
      editor_backspace(&editor);
    }

    if (IsKeyPressed(KEY_ENTER)) {
      editor_insert_new_line(&editor);
    }

    if (IsKeyPressed(KEY_DELETE)) {
      editor_delete(&editor);
    }

    if (IsKeyPressed(KEY_UP)) {
      if (editor.cursor_row > 0) {
        editor.cursor_row -= 1;
      }
    }

    if (IsKeyPressed(KEY_DOWN)) {
      editor.cursor_row += 1;
    }

    if (IsKeyPressed(KEY_LEFT)) {
      if (editor.cursor_col > 0) {
        editor.cursor_col -= 1;
      }
    }

    if (IsKeyPressed(KEY_RIGHT)) {
      editor.cursor_col += 1;
    }

    int key = GetCharPressed();

    if (key >= 32 && key <= 126) {
      editor_insert_text_before_cursor(&editor, (char *)&key);
    }

    BeginDrawing();
    ClearBackground(BLACK);

    for (size_t row = 0; row < editor.size; ++row) {
      const Line *line = editor.lines + row;
      DrawText(line->chars, 0, row * 12, 12, WHITE);
    }
    render_cursor();

    EndDrawing();
  }

  CloseWindow();

  return 0;
}
