#include "linenode.h"

#include <string.h>

LineNode *new_line(LineNode *parent) {
  LineNode *ln = malloc(sizeof(LineNode));
  ln->head = create_node();
  ln->next = NULL;
  ln->prev = parent;
}

void print_line_node(LineNode *ln) { print_node(ln->head); }

// disregards linenode boundaries, overflows to the next if there is not enough
// space
void line_node_append(LineNode *ln, const char *text) {
  insert_into_node(&ln->head, ln->head->size, text);
}

void line_node_delete(LineNode *ln, size_t index, size_t length) {
  if (index < ln->head->size) {
    delete_from_node(&ln->head, index, length);
  }
}

void line_node_replace(LineNode *ln, size_t index, const char *text) {
  if (index < ln->head->size) {
    // delete the portion of text that will be replaced
    size_t replace_len = strlen(text);
    size_t remaining_length = ln->head->size - index;

    // if replacement text is longer than remaining text, only delete up to the
    // length of the replacement text
    delete_from_node(&ln->head, index,
                     (replace_len < remaining_length) ? replace_len
                                                      : remaining_length);
  }

  insert_into_node(&ln->head, index, text);
}