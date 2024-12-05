#pragma once
#include "linenode.h"
#include <stdio.h>

#define DOCUMENT_INDEX_GAP 5

typedef struct {
  LineNode **index;
  size_t index_size;
  size_t line_gap;
} LineIndex;

typedef struct Document {
  LineNode *first_line;
  LineNode *last_line;
  size_t line_count;
  LineIndex line_index;
  char *name;
} Document;

void document_init(Document *d);
void document_deinit(Document *d);
void document_append(Document *d, const char *text);
void document_newline(Document *d);
void document_insert_newline(Document *doc, int line, int index);
void document_print(Document *d);
void document_print_structure(Document *d);
void document_build_index(Document *d, size_t gap);
LineNode *document_find_line(Document *d, int i);
void document_load_file(Document *d, char *filename);
void document_delete_line(Document *doc, int line);
void document_delete_char(Document *doc, int line, int col);