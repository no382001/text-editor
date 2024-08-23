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

typedef enum { INSERTION, DELETION } ModificationType;

Node *create_node();
void split_node(Node *node);
void merge_nodes(Node *node);
void modify_node(Node **head, size_t index, const char *str, size_t len,
                 ModificationType mod_type);
void insert_into_node(Node **head, size_t index, const char *str);
void delete_from_node(Node **head, size_t index, size_t length);
void delete_node(Node **head, size_t index, size_t length);
void print_node(Node *head);
void free_node(Node *head);