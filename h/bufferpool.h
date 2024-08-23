#pragma once
#include <stdbool.h>
#include <stdlib.h>

#define POOL_SIZE 128
#define BUFFER_SIZE 512

typedef struct {
  char *buffer;
  size_t size;
  bool in_use;
} BufferPoolItem;

typedef struct {
  BufferPoolItem *items;
  size_t count;
  size_t capacity;
} BufferPool;

void buffer_pool_init(size_t initial_capacity);
void buffer_pool_deinit();
char *buffer_pool_alloc(size_t size);
void buffer_pool_free(void *ptr);