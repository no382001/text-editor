#include "document.h"
#include "bufferpool.h"
#include "utils.h"

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

  log_message(DEBUG, "index built!\n");
  for (size_t j = 0; j < d->line_index.index_size; j++) {
    log_message(DEBUG, "index[%zu] -> %s\n", j,
                d->line_index.index[j]->head->chunk);
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

void document_print_structure(Document *d) {
  printf("Document Structure:\n");
  printf("Total lines: %zu\n", d->line_count);

  LineNode *ln = d->first_line;
  int line_number = 1;

  while (ln) {
    printf("Line %d [LineNode]:\n", line_number++);

    Node *node = ln->head;
    int node_number = 1;

    while (node) {
      printf("  Node %d [Node]: size = %zu, content = \"", node_number++,
             node->size);
      for (size_t i = 0; i < node->size; ++i) {
        printf("%c", node->chunk[i]);
      }
      printf("\"\n");
      node = node->next;
    }

    ln = ln->next;
  }

  printf("\nDocument Index Structure:\n");
  for (size_t i = 0; i < d->line_index.index_size; i++) {
    printf("Index %zu -> LineNode starting with: \"", i);
    Node *node = d->line_index.index[i]->head;
    if (node) {
      for (size_t j = 0; j < node->size; ++j) {
        printf("%c", node->chunk[j]);
      }
    }
    printf("\"\n");
  }
}