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

  buffer_pool_init(POOL_SIZE);

  Document d;
  document_init(&d);
  /**/
  document_append(&d, "first ");
  document_append(&d, "second");
  document_newline(&d);
  document_append(&d, "Hello ");
  document_append(&d, "Hello ");
  document_append(&d, "Hello World!");

  document_build_index(&d, 2);
  document_print(&d);

  LineNode *l1 = document_find_line(&d, 0);
  if (l1) {
    printf("----\n");
    print_node(l1->head);
  } else {
    printf("Line 1 not found.\n");
  }

  LineNode *l2 = document_find_line(&d, 1);
  if (l2) {
    printf("----\n");
    print_node(l2->head);
  } else {
    printf("Line 2 not found.\n");
  }

  line_node_append(
      l2,
      "This is appended after i searched for it i hope nothing bad happens");
  line_node_append(l2,
                   " , this is pretty long, almost 128, like the chunk size");

  printf("----\n");
  document_print_structure(&d);

  printf("----\n");

  delete_from_node(&l2->head, 12, 10);
  delete_from_node(&l2->head, 12, 30);

  printf("----\n");
  document_print_structure(&d);

  document_deinit(&d);
  buffer_pool_deinit();
  return 0;
}