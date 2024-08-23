#include "bufferpool.h"
#include "document.h"
#include "linenode.h"
#include "node.h"
#include <stdio.h>
#include <string.h>

int main() {
  buffer_pool_init(POOL_SIZE);

  Document d;
  document_init(&d);
  document_append(&d, "first ");
  document_append(&d, "second");
  document_newline(&d); // Line 1
  document_append(&d, "Hello ");
  document_append(&d, "Hello ");
  document_append(&d, "Hello World!");

  document_build_index(&d, 2);
  document_print(&d);

  LineNode *l1 = document_find_line(&d, 1);
  if (l1) {
    printf("----\n");
    print_node(l1->head);
  } else {
    printf("Line 1 not found.\n");
  }

  LineNode *l2 = document_find_line(&d, 2);
  if (l2) {
    printf("----\n");
    print_node(l2->head);
  } else {
    printf("Line 2 not found.\n");
  }

  document_deinit(&d);
  buffer_pool_deinit();
}