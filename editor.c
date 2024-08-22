#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define POOL_SIZE 128
#define BUFFER_SIZE 512

typedef struct {
    char* buffer;
    size_t size;
    bool in_use;
} BufferPoolItem;

typedef struct {
    BufferPoolItem* items;
    size_t count;
    size_t capacity;
} BufferPool;

BufferPool pool;

void buffer_pool_init(size_t initial_capacity) {
    pool.items = malloc(sizeof(BufferPoolItem) * initial_capacity);
    pool.count = 0;
    pool.capacity = initial_capacity;

    for (int i = 0; i < pool.capacity; i++){
        pool.items[i].buffer = (char*)malloc(BUFFER_SIZE);
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

char* buffer_pool_alloc(size_t size) {
    for (size_t i = 0; i < pool.count; ++i) {
        if (!pool.items[i].in_use && pool.items[i].size >= size) {
            pool.items[i].in_use = true;
            return pool.items[i].buffer;
        }
    }
}

void buffer_pool_free(void* ptr) {
    for (size_t i = 0; i < pool.count; ++i) {
        if (pool.items[i].buffer == ptr) {
            pool.items[i].in_use = false;
            return;
        }
    }
}

int main() {
    buffer_pool_init(POOL_SIZE);

    int size = 100;
    char* str = buffer_pool_alloc(size);
    memcpy(str,"asdadsdad",10);
    
    /*

    */

    buffer_pool_deinit();

    return 0;
}

