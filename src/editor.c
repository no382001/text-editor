#include "bufferpool.h"
#include "cursor.h"
#include "document.h"
#include "linenode.h"
#include "node.h"
#include "utils.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "raylib.h"
#include <math.h>

int scroll_offset = 0;
int font_size = 20;
int line_padding = 2;

extern Cursor cursor;

void draw(Document *d, Font *font) {
  LineNode *ln = d->first_line;
  int line_number = 0;

  while (ln) {
    Node *node = ln->head;
    int column = 0;
    int x = 10;
    bool cursor_drawn = false;

    while (node) {
      for (int i = 0; i < node->size; i++) {
        char character[2] = {node->chunk[i], '\0'};
        Vector2 position = {x, 200 + line_number * (font_size + line_padding) -
                                   scroll_offset};

        DrawTextEx(*font, character, position, font_size, 0, LIGHTGRAY);

        // check if this is where the cursor should be drawn
        if (line_number == cursor.line && column == cursor.column) {
          DrawRectangle(position.x, position.y, 2, font_size, RED);
          cursor_drawn = true;
        }

        x += MeasureTextEx(*font, character, font_size, 0).x;
        column++;
      }
      node = node->next;
    }

    // fix this!
    // if the cursor is at the end of the line, draw it after the last character
    if (line_number == cursor.line && !cursor_drawn) {
      Vector2 position = {x, 200 + line_number * (font_size + line_padding) -
                                 scroll_offset};
      DrawRectangle(position.x, position.y, 2, font_size, RED);
    }

    line_number++;
    ln = ln->next;
  }
}

int main() {

  const int screen_width = 1024;
  const int screen_height = 768;
  InitWindow(screen_width, screen_height, "tins");
  SetTargetFPS(60);

  Font fontTtf = LoadFontEx("Elronmonospace.ttf", 32, 0, 250);

  /* ---- */
  buffer_pool_init(POOL_SIZE);
  Document d;
  document_init(&d);
  print_path();
  document_load_file(&d, "src/node.c");
  document_build_index(&d, 5);

  /**/
  while (!WindowShouldClose()) {

    BeginDrawing();
    ClearBackground(BLACK);

    draw(&d, &fontTtf);
    handle_keys(&d);

    EndDrawing();
  }

  document_deinit(&d);
  buffer_pool_deinit();
  CloseAudioDevice();
  CloseWindow();
  /* */

  return 0;
}