#include "linenode.h"
#include "utils.h"
#include <string.h>

LineNode *new_line(LineNode *parent) {
  LineNode *ln = malloc(sizeof(LineNode));
  ln->head = create_node();
  chk_ptr(ln->head);
  ln->next = NULL;
  ln->prev = parent;
  return ln;
}

void print_line_node(LineNode *ln) { print_node(ln->head); }

// disregards linenode boundaries, overflows to the next if there is not enough
// space
void line_node_append(LineNode *ln, const char *text) {
  insert_into_node(&ln->head, ln->head->size, text);
}

void line_node_delete(LineNode *ln, size_t index, size_t length) {
  chk_ptr(ln->head);

  if (index < ln->head->size) {
    delete_from_node(&ln->head, index, length);
  }

  // if the entire line was deleted, free the line node
  if (ln->head == NULL || ln->head->size == 0) {
    // if there is a previous line, adjust the links
    if (ln->prev) {
      ln->prev->next = ln->next;
    }
    if (ln->next) {
      ln->next->prev = ln->prev;
    }

    free(ln);
    return;
  }

  // if the deletion started from index 0 and the line is not empty
  if (index == 0 && ln->head && ln->prev) {
    // Find the last node of the previous line
    Node *prev_node = ln->prev->head;
    while (prev_node->next) {
      prev_node = prev_node->next;
    }

    // merge the current line's head into the previous line's last node
    insert_into_node(&prev_node, prev_node->size, ln->head->chunk);

    // re-link
    if (ln->next) {
      ln->next->prev = ln->prev;
    }
    ln->prev->next = ln->next;

    free_node(ln->head);
    free(ln);
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

void line_node_insert_char(LineNode *ln, size_t index, char c) {
  modify_node(&ln->head, index, (char *)&c, 1, INSERTION);
}

void line_node_insert_newline(LineNode *ln, size_t index) {

  // find the node and the local index within that node where the 'split' should
  // occur
  Node *current_node = ln->head;
  size_t current_pos = 0;

  while (current_node && current_pos + current_node->size <= index) {
    current_pos += current_node->size;
    current_node = current_node->next;
  }

  // if the index is exactly at the end of the line, just create a new line
  if (!current_node || index == current_pos + current_node->size) {
    LineNode *new_ln = new_line(ln);
    new_ln->next = ln->next;
    if (ln->next) {
      ln->next->prev = new_ln;
    }
    ln->next = new_ln;
    new_ln->prev = ln;
    return;
  }

  // calculate the local index within the current node
  size_t local_index = index - current_pos;

  // now create a new line and transfer the content after the index to it
  LineNode *new_ln = new_line(ln);

  // copy the content after the local index to the new node
  size_t split_size = current_node->size - local_index;
  memcpy(new_ln->head->chunk, current_node->chunk + local_index, split_size);
  new_ln->head->size = split_size;

  current_node->size = local_index;

  // insert the new line after the current one
  new_ln->next = ln->next;
  if (ln->next) {
    ln->next->prev = new_ln;
  }
  ln->next = new_ln;
  new_ln->prev = ln;
}
