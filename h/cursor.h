#pragma once
#include "document.h"

typedef struct {
  int line;
  int column;
} Cursor;

void handle_keys(Document *d);