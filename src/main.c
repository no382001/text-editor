#include "bufferpool.h"
#include "document.h"
#include "editor.h"
#include "linenode.h"
#include "networking.h"
#include "node.h"
#include "utils.h"

#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

Document *g_d = 0;

int main() {
  /* ---- */
  buffer_pool_init(POOL_SIZE);
  Document d;
  document_init(&d);
  g_d = &d;
  print_path();
  document_load_file(&d, "src/node.c");
  document_build_index(&d, 5);

  networking_thread();

  document_deinit(&d);
  buffer_pool_deinit();

  return 0;
}