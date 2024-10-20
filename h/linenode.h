#pragma once
#include "node.h"

typedef struct LineNode {
  Node *head;
  struct LineNode *next;
  struct LineNode *prev;
} LineNode;

LineNode *new_line(LineNode *parent);
void print_line_node(LineNode *ln);
void line_node_append(LineNode *ln, const char *text);
void line_node_delete(LineNode *ln, size_t index, size_t length);
void line_node_replace(LineNode *ln, size_t index, const char *text);
void line_node_insert_char(LineNode *ln, size_t index, char c);
int line_node_size(LineNode *ln);