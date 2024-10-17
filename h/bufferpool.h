#pragma once
#include <stdbool.h>
#include <stdlib.h>

#define POOL_SIZE 128
#define BUFFER_SIZE 512

typedef struct {
  char *buffer;
  int size;
  bool in_use;
} BufferPoolItem;

typedef struct {
  BufferPoolItem *items;
  int used_count;
  int capacity;
} BufferPool;

void buffer_pool_init(int initial_capacity);
void buffer_pool_deinit();
char *buffer_pool_alloc(int size);
void buffer_pool_free(void *ptr);