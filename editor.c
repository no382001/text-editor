#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define POOL_SIZE 128
#define BUFFER_SIZE 512

#define CHUNK_SIZE 128
#define MERGE_THRESHOLD 32

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

BufferPool pool;

void buffer_pool_init(size_t initial_capacity) {
  pool.items = malloc(sizeof(BufferPoolItem) * initial_capacity);
  pool.count = 0;
  pool.capacity = initial_capacity;

  for (int i = 0; i < pool.capacity; i++) {
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
}

void buffer_pool_free(void *ptr) {
  for (size_t i = 0; i < pool.count; ++i) {
    if (pool.items[i].buffer == ptr) {
      pool.items[i].in_use = false;
      return;
    }
  }
}

#define CHUNK_SIZE 128
#define MIN_SIZE 32
#define SPLIT_SIZE 64

typedef struct Node {
  char *chunk;
  char size;
  struct Node *prev;
  struct Node *next;
} Node;

Node *create_node() {
  Node *node = (Node *)malloc(sizeof(Node));
  node->chunk = (char *)buffer_pool_alloc(CHUNK_SIZE);
  node->size = 0;
  node->prev = NULL;
  node->next = NULL;
  return node;
}

void split_node(Node *node) {
  Node *new_node = create_node();
  new_node->size = SPLIT_SIZE;

  memcpy(new_node->chunk, node->chunk + SPLIT_SIZE, SPLIT_SIZE);
  node->size = SPLIT_SIZE;

  new_node->next = node->next;
  if (new_node->next) {
    new_node->next->prev = new_node;
  }

  new_node->prev = node;
  node->next = new_node;
}

void merge_nodes(Node *node) {
  if (node->next && node->size + node->next->size <= CHUNK_SIZE) {
    Node *next_node = node->next;
    memcpy(node->chunk + node->size, next_node->chunk, next_node->size);
    node->size += next_node->size;

    node->next = next_node->next;
    if (node->next) {
      node->next->prev = node;
    }

    free(next_node);
  }
}

void insert(Node **head, size_t index, const unsigned char *str) {
  size_t len = strlen((const char *)str);
  size_t str_index = 0;

  while (str_index < len) {
    Node *node = *head;
    size_t current_index = 0;

    while (node && current_index + node->size <= index) {
      current_index += node->size;
      node = node->next;
    }

    if (!node) {
      node = create_node();
      if (*head == NULL) {
        *head = node;
      } else {
        Node *tail = *head;
        while (tail->next) {
          tail = tail->next;
        }
        tail->next = node;
        node->prev = tail;
      }
    }

    size_t local_index = index - current_index;
    size_t space_available = CHUNK_SIZE - node->size;
    size_t insert_length = len - str_index;

    if (insert_length > space_available) {
      insert_length = space_available;
    }

    memmove(node->chunk + local_index + insert_length,
            node->chunk + local_index, node->size - local_index);
    memcpy(node->chunk + local_index, str + str_index, insert_length);
    node->size += insert_length;
    str_index += insert_length;
    index += insert_length;

    if (node->size == CHUNK_SIZE) {
      split_node(node);
    }
  }
}

void delete (Node **head, size_t index, size_t length) {
  Node *node = *head;
  size_t current_index = 0;

  while (node && current_index + node->size <= index) {
    current_index += node->size;
    node = node->next;
  }

  while (node && length > 0) {
    size_t local_index = index - current_index;
    size_t available_to_delete = node->size - local_index;
    size_t to_delete =
        (length < available_to_delete) ? length : available_to_delete;

    memmove(node->chunk + local_index, node->chunk + local_index + to_delete,
            node->size - local_index - to_delete);
    node->size -= to_delete;
    length -= to_delete;

    Node *next = node->next; // store the next node before any potential free

    if (node->size < MIN_SIZE) {
      if (node->prev && node->prev->size + node->size <= CHUNK_SIZE) {
        merge_nodes(node->prev);
        node = node->prev; // after merging, continue with the previous node
      } else if (next && node->size + next->size <= CHUNK_SIZE) {
        merge_nodes(node);
        node = next; // if the current node was merged, move to the next node
      }
    }

    if (node->size == 0) {
      if (node->prev) {
        node->prev->next = node->next;
      } else {
        *head = node->next;
      }
      if (node->next) {
        node->next->prev = node->prev;
      }
      free(node);
      node = next; // move to the next node after freeing the current one
    } else {
      node = node->next;
    }

    current_index += node ? node->size : 0;
  }
}

void print_list(Node *head) {
  Node *node = head;
  while (node) {
    for (int i = 0; i < node->size; i++) {
      printf("%c", node->chunk[i]);
    }
    node = node->next;
  }
  printf("\n");
}

void free_list(Node *head) {
  Node *node = head;
  while (node) {
    Node *next = node->next;
    free(node);
    node = next;
  }
}

int main() {
  buffer_pool_init(POOL_SIZE);

  Node *head = create_node();
  insert(&head, 0, (unsigned char *)"Hello");
  insert(&head, 5, (unsigned char *)"This is a test of merge nodes.");
  print_list(head);

  // orce a split to create multiple nodes
  insert(&head, 35,
         (unsigned char *)" This will cause multiple nodes to be created.");
  print_list(head);

  // delete enough characters to cause a merge between nodes
  delete (&head, 5, 30); // remove part of "This is a test of merge nodes."
  print_list(
      head); // xxpect "Hello This will cause multiple nodes to be created."

  free_list(head);
  buffer_pool_deinit();
}