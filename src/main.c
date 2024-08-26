#include "bufferpool.h"
#include "cursor.h"
#include "document.h"
#include "editor.h"
#include "linenode.h"
#include "node.h"
#include "utils.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "raylib.h"
#include <math.h>

extern Cursor cursor;

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