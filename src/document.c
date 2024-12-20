#include "document.h"
#include "bufferpool.h"
#include "utils.h"

#include <stdio.h>

void document_init(Document *d) {
  d->first_line = new_line(NULL);
  chk_ptr(d->first_line);
  d->last_line = d->first_line;
  d->line_count = 1;

  d->line_index.index = NULL;
  d->line_index.index_size = 0;
  d->line_index.line_gap = 0;
}

void document_deinit(Document *d) {
  if (!d) {
    return;
  }

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

void document_reset(Document *d) {
  log_message(DEBUG, "document reset!");
  document_deinit(d);
  document_init(d);
}

void document_append(Document *d, const char *text) {
  line_node_append(d->last_line, text);
}

void document_newline(Document *d) {
  d->last_line->next = new_line(d->last_line);
  chk_ptr(d->last_line->next);
  d->last_line = d->last_line->next;
  d->line_count++;
}

void document_print(Document *d) {
  LineNode *ln = d->first_line;
  while (ln) {
    print_node(ln->head);
    ln = ln->next;
    printf("\n");
  }
}

void document_build_index(Document *d, size_t gap) {
  if (d->line_index.index) {
    free(d->line_index.index);
  }

  d->line_index.line_gap = gap;

  size_t max_index_size = ((d->line_count + gap - 1) / gap);
  d->line_index.index = malloc(max_index_size * sizeof(LineNode *));

  LineNode *ln = d->first_line;
  size_t idx = 0;
  size_t line_number = 0;

  while (ln) {
    if (line_number % gap == 0) {
      d->line_index.index[idx++] = ln;
    }
    ln = ln->next;
    line_number++;
  }

  d->line_index.index_size = idx;

  log_message(DEBUG, "index built with %zu entries!", d->line_index.index_size);

  // printf("\033[H\033[J");
  // fflush(stdout);
  // document_print_structure(d);
}

LineNode *document_find_line(Document *d, int i) {
  if (!d->line_index.index) {
    log_message(ERROR, "doc index was not built, cant search for line");
    return NULL;
  }
  // i--; // 0 indexing is confusing in this case

  if (i < 0 || i > d->line_count) {
    return NULL;
  }

  LineIndex *li = &d->line_index;
  size_t idx = i / li->line_gap;

  if (idx >= li->index_size || idx < 0) {
    return NULL;
  }

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
      printf("  Node %d [Node]: size = %ld, rsize = %ld, content = \"",
             node_number++, node->size, strlen(node->chunk));
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

void document_load_file(Document *d, char *filename) {
  FILE *file = fopen(filename, "r");
  if (!file) {
    perror("Failed to open file");
    return;
  }

  if (d) {
    document_reset(d);
  }

  d->name = filename;

  char *line = NULL;
  size_t len = 0;
  size_t read;

  while ((read = getline(&line, &len, file)) != -1) {
    if (line[read - 1] == '\n') {
      line[read - 1] = '\0';
    }
    document_append(d, line);
    document_newline(d);
  }

  free(line);
  fclose(file);
}

void document_delete_line(Document *doc, int line) {
  LineNode *ln = document_find_line(doc, line);
  if (!ln) {
    return;
  }
  LineNode *next = ln->next;
  line_node_delete(ln, 0, line_node_size(ln));
  if (line == 0) {
    doc->first_line = next;
  }

  doc->line_count--;
}

void document_delete_char(Document *doc, int line, int col) {
  if (col == 0 && line == 0) {
    return;
  } else if (col == 0 && line > 0) {
    LineNode *current_line = document_find_line(doc, line);
    LineNode *previous_line = document_find_line(doc, line - 1);

    if (!current_line || !previous_line) {
      return;
    }

    Node *head = current_line->head;

    while (head) {
      char buff[CHUNK_SIZE];
      snprintf(buff, CHUNK_SIZE, "%.*s", (int)head->size, head->chunk);
      line_node_append(previous_line, buff); // this is sketchy
      head = head->next;
    }

    // at this point `current` line should have been appended to the `prev` one
    // so thats why you delete
    document_delete_line(doc, line);
  } else {
    line_node_delete(document_find_line(doc, line), col, 1);
  }
}

void document_insert_newline(Document *doc, int line, int index) {
  LineNode *ln = document_find_line(doc, line);
  Node *current_node = ln->head;
  int current_pos = 0;

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

    doc->line_count++;
    return;
  }

  // calculate the local index within the current node
  int local_index = index - current_pos;

  // now create a new line and transfer the content after the index to it
  LineNode *new_ln = new_line(ln);

  // copy the content after the local index to the new node
  int split_size = current_node->size - local_index;
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

  doc->line_count++;
}