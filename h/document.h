#pragma once
#include "linenode.h"

typedef struct Document {
  LineNode *first_line;
  LineNode *last_line;
  size_t line_count;
} Document;

void document_init(Document *d);
void document_deinit(Document *d);
void document_append(Document *d, const char *text);
void document_newline(Document *d);
void document_print(Document *d);