#include "bufferpool.h"
#include "document.h"
#include "linenode.h"
#include "node.h"
#include "utils.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "raylib.h"
#include <math.h>

#include <limits.h>
#include <unistd.h>

void print_path() {
  char cwd[PATH_MAX];

  if (getcwd(cwd, sizeof(cwd)) != NULL) {
    printf("Current working directory: %s\n", cwd);
  } else {
    perror("getcwd() error");
  }
}

int scroll_offset = 0;
int font_size = 20;
int line_padding = 2;

void handle_keys() {
  if (IsKeyPressed(KEY_BACKSPACE)) {
  }
  if (IsKeyPressed(KEY_ENTER)) {
  }
  if (IsKeyPressed(KEY_DELETE)) {
  }
  if (IsKeyPressed(KEY_UP)) {
    scroll_offset -= font_size + line_padding;
    if (scroll_offset < 0) {
      scroll_offset = 0;
    }
  }
  if (IsKeyPressed(KEY_DOWN)) {
    scroll_offset += font_size + line_padding;
  }
  if (IsKeyPressed(KEY_LEFT)) {
  }
  if (IsKeyPressed(KEY_RIGHT)) {
  }
  if (IsKeyPressed(KEY_PAGE_UP)) {
    scroll_offset -= (font_size + line_padding) * 10;
    if (scroll_offset < 0) {
      scroll_offset = 0;
    }
  }
  if (IsKeyPressed(KEY_PAGE_DOWN)) {
    scroll_offset += (font_size + line_padding) * 10;
  }
  int key = GetCharPressed();
  if (key > 0) {
    //
  }
}

void draw_text(Document *d) {
  LineNode *ln = d->first_line;
  int i = 0;
  while (ln) {
    Node *node = ln->head;
    while (node) {
      char buff[128];
      sprintf(buff, "%.*s", node->size, node->chunk);

      DrawText(buff, 20, 200 + i++ * (font_size + line_padding) - scroll_offset,
               font_size, LIGHTGRAY);
      node = node->next;
    }
    ln = ln->next;
  }
}

int main() {

  const int screen_width = 1024;
  const int screen_height = 768;
  InitWindow(screen_width, screen_height, "tins");
  SetTargetFPS(60);

  /* ---- */
  buffer_pool_init(POOL_SIZE);
  Document d;
  document_init(&d);
  print_path();
  document_load_file(&d, "src/node.c");
  document_build_index(&d, 5);

  document_print(&d);

  /**/
  while (!WindowShouldClose()) {

    BeginDrawing();
    ClearBackground(BLACK);

    draw_text(&d);
    handle_keys();

    EndDrawing();
  }

  document_deinit(&d);
  buffer_pool_deinit();
  CloseAudioDevice();
  CloseWindow();
  /* */

  return 0;
}