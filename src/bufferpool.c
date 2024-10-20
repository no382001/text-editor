#include "bufferpool.h"
#include "utils.h"
#include <assert.h>

//#undef log_message
//#define log_message(level, message, ...)

/*

BufferPool pre-allocates memory for the strings reducing syscalls significantly

*/

BufferPool pool;

void buffer_pool_init(int initial_capacity) {
  pool.items = malloc(sizeof(BufferPoolItem) * initial_capacity);
  pool.used_count = 0;
  pool.capacity = initial_capacity;

  for (int i = 0; i < pool.capacity; i++) {
    // this doesnt seem right BUFFER_SIZE for sure? it POOL_SIZE is the max for
    // a split e.g.
    pool.items[i].buffer = (char *)calloc(BUFFER_SIZE, sizeof(char));
    chk_ptr(pool.items[i].buffer);
    pool.items[i].size = BUFFER_SIZE;
    pool.items[i].in_use = false;
  }
}

void buffer_pool_deinit() {
  for (size_t i = 0; i < pool.capacity; ++i) {
    if (pool.items[i].buffer) {
      free(pool.items[i].buffer);
      pool.items[i].buffer = NULL;
    }
  }
  free(pool.items);
}

char *buffer_pool_alloc(int size) {
  assert(size <= BUFFER_SIZE);
  for (int i = 0; i < pool.capacity; ++i) {
    if (!pool.items[i].in_use && pool.items[i].size >= size) {
      log_message(DEBUG, "[1] buffer pool allocation pool.in_use: %d of %d",
                  pool.used_count, pool.capacity);
      pool.items[i].in_use = true;
      pool.used_count++;
      assert(pool.items[i].buffer);
      return pool.items[i].buffer;
    }
  }
  log_message(WARNING, "[1] bufferpool is out of memory cap:%d", pool.capacity);

  // there was no more space, so do this
  size_t new_capacity = pool.capacity * 2;
  log_message(WARNING, "[2] reallocating space... new cap:%d", new_capacity);

  BufferPoolItem *new_items =
      realloc(pool.items, sizeof(BufferPoolItem) * new_capacity);
  if (!new_items) {
    // thats a problem
    log_message(ERROR, "[!] reallocating space... failed");
    return NULL;
  }

  pool.items = new_items;

  for (size_t i = pool.capacity; i < new_capacity; ++i) {
    pool.items[i].buffer = (char *)calloc(BUFFER_SIZE, sizeof(char));
    pool.items[i].size = BUFFER_SIZE;
    pool.items[i].in_use = false;
  }

  pool.capacity = new_capacity;

  log_message(DEBUG, "[3] retrying allocation after realloc");
  return buffer_pool_alloc(size); // retry
}

void buffer_pool_free(void *ptr) {
  for (size_t i = 0; i < pool.capacity; ++i) {
    if (pool.items[i].buffer == ptr) {
      pool.items[i].in_use = false;
      pool.used_count--;
      log_message(DEBUG, "buffer pool free pool.in_use: %d", pool.used_count);
      return;
    }
  }
}