#include "bufferpool.h"
#include "utils.h"

/*

BufferPool pre-allocates memory for the strings reducing syscalls significantly

*/

BufferPool pool;

void buffer_pool_init(size_t initial_capacity) {
  pool.items = malloc(sizeof(BufferPoolItem) * initial_capacity);
  pool.count = 0;
  pool.capacity = initial_capacity;

  for (int i = 0; i < pool.capacity; i++) {
    // this doesnt seem right BUFFER_SIZE for sure? it POOL_SIZE is the max for
    // a split e.g.
    pool.items[i].buffer = (char *)malloc(BUFFER_SIZE);
    pool.items[i].size = BUFFER_SIZE;
    pool.items[i].in_use = false;
    pool.count++;
  }
}

void buffer_pool_deinit() {
  for (size_t i = 0; i < pool.count; ++i) {
    free(pool.items[i].buffer);
  }
  free(pool.items);
}

char *buffer_pool_alloc(size_t size) {
  for (size_t i = 0; i < pool.count; ++i) {
    if (!pool.items[i].in_use && pool.items[i].size >= size) {
      pool.items[i].in_use = true;
      return pool.items[i].buffer;
    }
  }
  log_message(WARNING, "bufferpool is out of memory cap:%d\n", pool.count);
  log_message(WARNING, "reallocating space... new cap:%d\n", pool.capacity * 2);

  // there was no more space, so do this
  size_t new_capacity = pool.capacity * 2;
  BufferPoolItem *new_items =
      realloc(pool.items, sizeof(BufferPoolItem) * new_capacity);
  if (!new_items) {
    // thats a problem
    return NULL;
  }

  pool.items = new_items;
  pool.capacity = new_capacity;

  for (size_t i = pool.count; i < pool.capacity; ++i) {
    pool.items[i].buffer = (char *)malloc(BUFFER_SIZE);
    pool.items[i].size = BUFFER_SIZE;
    pool.items[i].in_use = false;
  }

  pool.count = pool.capacity;

  return buffer_pool_alloc(size); // retry
}

void buffer_pool_free(void *ptr) {
  for (size_t i = 0; i < pool.count; ++i) {
    if (pool.items[i].buffer == ptr) {
      pool.items[i].in_use = false;
      return;
    }
  }
}