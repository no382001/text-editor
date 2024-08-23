#include "document.h"
#include "bufferpool.h"
#include <stdio.h>

void document_init(Document *d) {
  d->first_line = new_line(NULL);
  d->last_line = d->first_line;
  d->line_count = 1;

  d->line_index.index = NULL;
  d->line_index.index_size = 0;
  d->line_index.line_gap = 0;
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
  free(d->line_index.index);
}

void document_append(Document *d, const char *text) {
  line_node_append(d->last_line, text);
}

void document_newline(Document *d) {
  d->last_line->next = new_line(d->last_line);
  d->last_line = d->last_line->next;
  d->line_count++;
}

void document_print(Document *d) {
  LineNode *ln = d->first_line;
  while (ln) {
    print_node(ln->head);
    ln = ln->next;
  }
}

void document_build_index(Document *d, size_t gap) {
  if (d->line_index.index) {
    free(d->line_index.index);
  }

  d->line_index.line_gap = gap;
  d->line_index.index_size = (d->line_count + gap - 1) / gap;
  d->line_index.index = malloc(d->line_index.index_size * sizeof(LineNode *));

  LineNode *ln = d->first_line;
  size_t idx = 0;
  size_t line_number = 0;

  while (ln && idx < d->line_index.index_size) {
    if (line_number % gap == 0) {
      d->line_index.index[idx++] = ln;
    }
    ln = ln->next;
    line_number++;
  }

  printf("[INFO] index built!\n");
  for (size_t j = 0; j < d->line_index.index_size; j++) {
    printf("[INFO] index[%zu] -> %s\n", j, d->line_index.index[j]->head->chunk);
  }
}

LineNode *document_find_line(Document *d, int i) {
  i--; // 0 indexing is confusing in this case

  if (i < 0 || i > d->line_count) {
    return NULL;
  }

  LineIndex *li = &d->line_index;
  size_t idx = i / li->line_gap;
  LineNode *ln = li->index[idx];
  size_t start_line = idx * li->line_gap;

  while (ln && start_line < i) {
    ln = ln->next;
    start_line++;
  }

  return ln;
}