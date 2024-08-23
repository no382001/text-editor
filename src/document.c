#include "document.h"
#include "bufferpool.h"

void document_init(Document *d) {
  d->first_line = new_line(NULL);
  d->last_line = d->first_line;
  d->line_count = 1;
}

void document_deinit(Document *d) {
  LineNode *current_line = d->first_line;
  while (current_line) {
    Node *current_node = current_line->head;
    while (current_node) {
      Node *next_node = current_node->next;
      buffer_pool_free(current_node->chunk);
      free(current_node);
      current_node = next_node;
    }

    LineNode *next_line = current_line->next;
    free(current_line);
    current_line = next_line;
  }

  d->first_line = NULL;
  d->last_line = NULL;
  d->line_count = 0;
}

void document_append(Document *d, const char *text) {
  line_node_append(d->last_line, text);
}

void document_newline(Document *d) {
  d->last_line->next = new_line(d->last_line);
  d->last_line = d->last_line->next;
}

void document_print(Document *d) {
  LineNode *ln = d->first_line;
  while (ln) {
    print_node(ln->head);
    ln = ln->next;
  }
}