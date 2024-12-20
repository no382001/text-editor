#include "bufferpool.h"
#include "document.h"
#include "editor.h"
#include "linenode.h"
#include "networking.h"
#include "node.h"
#include "utils.h"

#include <math.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

Document *g_d = 0;

int main() {
  set_log_level(DEBUG);
  /* ---- */
  buffer_pool_init(POOL_SIZE);
  Document d = {0};
  document_init(&d);
  g_d = &d;

  network_cfg_t n = {.startup_cmd = "echo open TODO"};
  networking_thread(&n);

  document_deinit(&d);
  buffer_pool_deinit();

  return 0;
}