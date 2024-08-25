#include "node.h"
#include "bufferpool.h"
#include "utils.h"

#include <stdio.h>
#include <string.h>

/*

Merging tree structure to store the strings themselves

*/

Node *create_node() {
  Node *node = (Node *)malloc(sizeof(Node));
  node->chunk = (char *)buffer_pool_alloc(CHUNK_SIZE);
  node->size = 0;
  node->prev = NULL;
  node->next = NULL;
  return node;
}

void split_node(Node *node) {
  log_message(DEBUG, "node split!\n");
  Node *new_node = create_node();
  new_node->size = SPLIT_SIZE;

  memcpy(new_node->chunk, node->chunk + SPLIT_SIZE, SPLIT_SIZE);
  node->size = SPLIT_SIZE;
  // null term?

  new_node->next = node->next;
  if (new_node->next) {
    new_node->next->prev = new_node;
  }

  new_node->prev = node;
  node->next = new_node;
}

void free_node(Node *n) {
  buffer_pool_free(n->chunk);
  free(n);
}

void merge_nodes(Node *node) {
  if (node->next && node->size + node->next->size <= CHUNK_SIZE) {
    log_message(DEBUG, "node merge!\n");
    Node *next_node = node->next;
    memcpy(node->chunk + node->size, next_node->chunk, next_node->size);
    // null term?

    node->size += next_node->size;

    node->next = next_node->next;
    if (node->next) {
      node->next->prev = node;
    }

    free_node(next_node);
  }
}

void modify_node(Node **head, size_t index, const char *str, size_t len,
                 ModificationType mod_type) {
  size_t str_index = 0;

  while (str_index < len || (mod_type == DELETION && len > 0)) {
    Node *node = *head;
    size_t current_index = 0;

    // traverse to find the correct node and index within the node
    while (node && current_index + node->size < index) {
      current_index += node->size;
      node = node->next;
    }

    if (!node) {
      // if no node is found, create a new node (for insertion)
      if (mod_type == INSERTION) {
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
      } else {
        // if deletion reaches beyond available nodes, break
        break;
      }
    }

    size_t local_index = index - current_index;
    size_t space_available = CHUNK_SIZE - node->size;

    if (mod_type == INSERTION) {
      // calculate how much we can insert in this node
      size_t insert_length = len - str_index;
      if (insert_length > space_available) {
        insert_length = space_available;
      }

      // move existing data to make space for new data
      memmove(node->chunk + local_index + insert_length,
              node->chunk + local_index, node->size - local_index);
      memcpy(node->chunk + local_index, str + str_index, insert_length);
      node->size += insert_length;
      // null term?

      str_index += insert_length;
      index += insert_length;

      if (node->size >= CHUNK_SIZE) {
        split_node(node);
      }
    } else if (mod_type == DELETION) {
      // calculate how much we can delete in this node
      size_t delete_length = len;
      if (local_index + delete_length > node->size) {
        delete_length = node->size - local_index;
      }

      // shift data to remove the specified characters
      memmove(node->chunk + local_index,
              node->chunk + local_index + delete_length,
              node->size - local_index - delete_length);
      node->size -= delete_length;
      len -= delete_length;

      // if the node is too small, attempt to merge with adjacent nodes
      if (node->size < MIN_SIZE) {
        if (node->prev && node->prev->size + node->size <= CHUNK_SIZE) {
          merge_nodes(node->prev);
          node = node->prev;
        } else if (node->next && node->size + node->next->size <= CHUNK_SIZE) {
          merge_nodes(node);
        }
      }

      if (node->size == 0) {
        // remove the node if it's empty
        if (node->prev) {
          node->prev->next = node->next;
        } else {
          *head = node->next;
        }
        if (node->next) {
          node->next->prev = node->prev;
        }
        free_node(node);
      }
    }
  }
}

void insert_into_node(Node **head, size_t index, const char *str) {
  modify_node(head, index, str, strlen(str), INSERTION);
}

void delete_from_node(Node **head, size_t index, size_t length) {
  modify_node(head, index, NULL, length, DELETION);
}

void delete_node(Node **head, size_t index, size_t length) {
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

void print_node(Node *head) {
  Node *node = head;
  while (node) {
    for (int i = 0; i < node->size; i++) {
      printf("%c", node->chunk[i]);
    }
    node = node->next;
  }
}