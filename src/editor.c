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

void handle_keys() {
  if (IsKeyPressed(KEY_BACKSPACE)) {
  }
  if (IsKeyPressed(KEY_ENTER)) {
  }
  if (IsKeyPressed(KEY_DELETE)) {
  }
  if (IsKeyPressed(KEY_UP)) {
  }
  if (IsKeyPressed(KEY_DOWN)) {
  }
  if (IsKeyPressed(KEY_LEFT)) {
  }
  if (IsKeyPressed(KEY_RIGHT)) {
  }
  int key = GetCharPressed();
  if (key > 0) {
  }
}

void draw_text(Document *d) {
  LineNode *ln = d->first_line;
  int i = 0;
  while (ln) {
    Node *node = ln->head;
    while (node) {
      char buff[128];
      sprintf(buff, node->chunk);
      DrawText(buff, 190, 200 + i++ * 2, 20, LIGHTGRAY);
      node = node->next;
    }
    ln = ln->next;
  }
}

int main() {

  const int screen_width = 1024;
  const int screen_height = 768;
  //InitWindow(screen_width, screen_height, "tins");
  //SetTargetFPS(60);

  /* ---- */
  buffer_pool_init(POOL_SIZE);
  Document d;
  document_init(&d);
  print_path();
  document_load_file(&d, "src/node.c");
  document_build_index(&d, 5);

  //document_print_structure(&d);
  /** /
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