#pragma once
#include <stdlib.h>

#define CHUNK_SIZE 128
#define MIN_SIZE 32
#define SPLIT_SIZE 64

typedef struct Node {
  char *chunk;
  size_t size;
  struct Node *prev;
  struct Node *next;
} Node;

Node *create_node();
void split_node(Node *node);
void merge_nodes(Node *node);
void insert_into_node(Node **head, size_t index, const unsigned char *str);
void delete_node(Node **head, size_t index, size_t length);
void print_node(Node *head);
void free_node(Node *head);