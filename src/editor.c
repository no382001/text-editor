#include <string.h>

#include "bufferpool.h"
#include "document.h"
#include "linenode.h"
#include "node.h"

int main() {
  buffer_pool_init(POOL_SIZE);

  Document d;
  document_init(&d);
  document_append(&d, "first ");
  document_append(&d, "second");
  document_newline(&d);
  document_append(&d, "Hello ");
  document_append(&d, "Hello ");
  document_append(&d, "Hello World!");

  document_print(&d);

  document_deinit(&d);
  buffer_pool_deinit();
}