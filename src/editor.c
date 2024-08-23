#include "bufferpool.h"
#include "document.h"
#include "linenode.h"
#include "node.h"
#include "utils.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

int main() {
  buffer_pool_init(POOL_SIZE);

  Document d;
  document_init(&d);
  for (int i = 0; i < POOL_SIZE + POOL_SIZE;
       i++) { // Allocate more than POOL_SIZE buffers
    document_append(&d,
                    "This is appended after i searched for it i hope nothing "
                    "bad happens , this is pretty long, almost 128, like the "
                    "chunk size, i hope it wont go in another");
    document_newline(&d);
  }

  document_build_index(&d, 5);

  document_deinit(&d);
  buffer_pool_deinit();
}