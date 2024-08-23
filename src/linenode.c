#include "linenode.h"

LineNode *new_line(LineNode *parent) {
  LineNode *ln = malloc(sizeof(LineNode));
  ln->head = create_node();
  ln->next = NULL;
  ln->prev = parent;
}

void line_node_append(LineNode *ln, const char *text) {
  insert_into_node(&ln->head, ln->head->size, text);
}