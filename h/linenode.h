#pragma once
#include "node.h"

typedef struct LineNode {
  Node *head;
  struct LineNode *next;
  struct LineNode *prev;
} LineNode;

LineNode *new_line(LineNode *parent);
void line_node_append(LineNode *ln, const char *text);