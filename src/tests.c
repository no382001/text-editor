
#include "unity.h"
#include "utils.h"

#define TEST_ASSERT_EQUAL_STRING_SIZE(expected, actual, size)                  \
  do {                                                                         \
    char actual_buffer[size + 1];                                              \
    snprintf(actual_buffer, size + 1, "%.*s", (int)size, actual);              \
    TEST_ASSERT_EQUAL_STRING(expected, actual_buffer);                         \
  } while (0)

/*  buffer pool */

#include "bufferpool.h"
//#undef POOL_SIZE
//#define POOL_SIZE 2

extern BufferPool pool;

void test_buffer_pool_init(void) {
  TEST_ASSERT_EQUAL(POOL_SIZE, pool.capacity);
  TEST_ASSERT_EQUAL(0, pool.used_count);

  for (size_t i = 0; i < pool.capacity; ++i) {
    TEST_ASSERT_NOT_NULL(pool.items[i].buffer);
    TEST_ASSERT_EQUAL(BUFFER_SIZE, pool.items[i].size);
    TEST_ASSERT_FALSE(pool.items[i].in_use);
  }
}

void test_buffer_pool_alloc(void) {
  char *buf1 = buffer_pool_alloc(256);
  TEST_ASSERT_NOT_NULL(buf1);
  TEST_ASSERT_EQUAL(1, pool.used_count);

  char *buf2 = buffer_pool_alloc(256);
  TEST_ASSERT_NOT_NULL(buf2);
  TEST_ASSERT_EQUAL(2, pool.used_count);

  char *buf3 = buffer_pool_alloc(256);
  TEST_ASSERT_NOT_NULL(buf3);
  TEST_ASSERT_EQUAL(3, pool.used_count);
}

void test_buffer_pool_free(void) {
  char *buf1 = buffer_pool_alloc(256);
  char *buf2 = buffer_pool_alloc(256);

  buffer_pool_free(buf1);
  TEST_ASSERT_EQUAL(1, pool.used_count);
  TEST_ASSERT_FALSE(pool.items[0].in_use);

  buffer_pool_free(buf2);
  TEST_ASSERT_EQUAL(0, pool.used_count);
  TEST_ASSERT_FALSE(pool.items[1].in_use);
}

void test_buffer_pool_realloc(void) {
  char *buf1 = buffer_pool_alloc(256);
  char *buf2 = buffer_pool_alloc(256);
  char *buf3 = buffer_pool_alloc(256); // realloc trigger

  TEST_ASSERT_EQUAL(POOL_SIZE, pool.capacity);
  TEST_ASSERT_EQUAL(3, pool.used_count);

  TEST_ASSERT_NOT_NULL(buf1);
  TEST_ASSERT_NOT_NULL(buf2);
  TEST_ASSERT_NOT_NULL(buf3);
}

/* node */

#include "node.h"
#include <string.h>

void test_create_node(void) {
  Node *node = create_node();

  TEST_ASSERT_NOT_NULL(node);
  TEST_ASSERT_NOT_NULL(node->chunk);
  TEST_ASSERT_EQUAL(0, node->size);
  TEST_ASSERT_NULL(node->prev);
  TEST_ASSERT_NULL(node->next);

  free_node(node);
}

void test_split_node(void) {
  Node *node = create_node();
  const char *data = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
  memcpy(node->chunk, data, SPLIT_SIZE * 2); // filling node with data
  node->size = SPLIT_SIZE * 2;

  split_node(node);

  TEST_ASSERT_EQUAL(SPLIT_SIZE, node->size);
  TEST_ASSERT_NOT_NULL(node->next);
  TEST_ASSERT_EQUAL(SPLIT_SIZE, node->next->size);

  TEST_ASSERT_EQUAL_MEMORY(data, node->chunk, SPLIT_SIZE);
  TEST_ASSERT_EQUAL_MEMORY(data + SPLIT_SIZE, node->next->chunk, SPLIT_SIZE);

  free_node(node->next);
  free_node(node);
}

void test_merge_nodes(void) {
  Node *node1 = create_node();
  Node *node2 = create_node();

  const char *data1 = "Hello, ";
  const char *data2 = "World!";

  memcpy(node1->chunk, data1, strlen(data1));
  node1->size = strlen(data1);

  memcpy(node2->chunk, data2, strlen(data2));
  node2->size = strlen(data2);

  node1->next = node2;
  node2->prev = node1;

  merge_nodes(node1);

  TEST_ASSERT_EQUAL(strlen(data1) + strlen(data2), node1->size);
  TEST_ASSERT_EQUAL_STRING("Hello, World!", node1->chunk);
  TEST_ASSERT_NULL(node1->next);

  free_node(node1);
}

void test_insert_into_node(void) {
  Node *head = create_node();
  insert_into_node(&head, 0, "Hello, World!");

  TEST_ASSERT_EQUAL(13, head->size);
  TEST_ASSERT_EQUAL_STRING_SIZE("Hello, World!", head->chunk, head->size);

  insert_into_node(&head, 7, "Cruel ");
  TEST_ASSERT_EQUAL_STRING_SIZE("Hello, Cruel World!", head->chunk, head->size);
  // ^^^^^^^

  free_node(head);
}

void test_delete_from_node(void) {
  Node *head = create_node();
  insert_into_node(&head, 0, "Hello, Cruel World!");
  delete_from_node(&head, 7, 6);

  TEST_ASSERT_EQUAL_STRING_SIZE("Hello, World!", head->chunk, head->size);
  TEST_ASSERT_EQUAL(13, head->size);

  delete_from_node(&head, 0, 6);
  TEST_ASSERT_EQUAL_STRING_SIZE(" World!", head->chunk, head->size);
  TEST_ASSERT_EQUAL(7, head->size);

  free_node(head);
}

void test_delete_from_node_wrong_index(void) {
  Node *head = create_node();
  insert_into_node(&head, 0, "Hello, Cruel World!");
  delete_from_node(&head, 7, 6);

  TEST_ASSERT_EQUAL_STRING_SIZE("Hello, World!", head->chunk, head->size);
  TEST_ASSERT_EQUAL(13, head->size);

  delete_from_node(&head, 0, 16); // from x to unbound
  TEST_ASSERT_EQUAL(0, head);

  free(head);
  head = NULL;

  head = create_node();
  insert_into_node(&head, 0, "Hello, World!");
  TEST_ASSERT_EQUAL(13, head->size);
  delete_from_node(&head, 14, 1); // from unbound to unbound
  TEST_ASSERT_EQUAL(true, head != 0);

  delete_from_node(&head, 30, 1); // from unbound to unbound
  TEST_ASSERT_EQUAL(true, head != 0);

  insert_into_node(&head, 30, "Hello, World!"); // to unbound
  TEST_ASSERT_EQUAL(13, head->size);
  free(head);
}

void test_delete_and_merge(void) {
  Node *head = create_node();
  insert_into_node(&head, 0,
                   "Hello,lk;asdl;askd;asdl;askdl;akl;kdl;askl;dkasl; ");
  insert_into_node(
      &head, 7,
      "World!"
      "opasopdiasopdiopasidpoasidpoasiopdiaspodiopasidpaosidpoasidpoasidpoasido"
      "psaidopasidposiaopdiaspodiasopdiasopdiopaidopis");

  delete_from_node(&head, 7, 1); // remove space shouldnt trigger merge
  delete_from_node(&head, 5, 60);
  delete_from_node(&head, 5, 60); // merge trigger, how do i test this?

  TEST_ASSERT_EQUAL_STRING_SIZE(
      "Helloidopisk;asdl;askd;asdl;askdl;akl;kdl;askl;dkasl; ", head->chunk,
      head->size);
  TEST_ASSERT_EQUAL(54, head->size);

  free_node(head);
}

void test_complex_modifications(void) {
  Node *head = create_node();
  insert_into_node(&head, 0, "Hello World!");

  insert_into_node(&head, 5, ", how are you?");
  TEST_ASSERT_EQUAL_STRING_SIZE("Hello, how are you? World!", head->chunk,
                                head->size);

  delete_from_node(&head, 7, 13); // deleting "how are you? "
  TEST_ASSERT_EQUAL_STRING_SIZE("Hello, World!", head->chunk, head->size);

  free_node(head);
}

/* linenode */

#include "linenode.h"

void test_new_line(void) {
  LineNode *ln = new_line(NULL);

  TEST_ASSERT_NOT_NULL(ln);
  TEST_ASSERT_NOT_NULL(ln->head);
  TEST_ASSERT_EQUAL(0, ln->head->size);
  TEST_ASSERT_NULL(ln->next);
  TEST_ASSERT_NULL(ln->prev);

  free_node(ln->head);
  free(ln);
}

void test_line_node_append(void) {
  LineNode *ln = new_line(NULL);
  line_node_append(ln, "Hello, World!");

  TEST_ASSERT_EQUAL_STRING_SIZE("Hello, World!", ln->head->chunk,
                                ln->head->size);
  TEST_ASSERT_EQUAL(strlen("Hello, World!"), ln->head->size);

  free_node(ln->head);
  free(ln);
}

void test_line_node_delete(void) {
  LineNode *ln = new_line(NULL);
  line_node_append(ln, "Hello, World!");

  // delete ", World"
  line_node_delete(ln, 5, 7);
  TEST_ASSERT_EQUAL_STRING_SIZE("Hello!", ln->head->chunk, ln->head->size);
  TEST_ASSERT_EQUAL(strlen("Hello!"), ln->head->size);

  free_node(ln->head);
  free(ln);
}

void test_line_node_replace(void) {
  LineNode *ln = new_line(NULL);
  line_node_append(ln, "Hello, World ! !");

  // replace "World" with "Universe"
  line_node_replace(ln, 7, "Universe");
  TEST_ASSERT_EQUAL_STRING_SIZE("Hello, Universe!", ln->head->chunk,
                                ln->head->size);
  TEST_ASSERT_EQUAL(strlen("Hello, Universe!"), ln->head->size);

  free_node(ln->head);
  free(ln);
}

void test_line_node_insert(void) {
  LineNode *ln = new_line(NULL);
  line_node_append(ln, "Helo, World!");

  // insert 'l' at position 2
  line_node_insert_char(ln, 2, 'l');
  TEST_ASSERT_EQUAL_STRING_SIZE("Hello, World!", ln->head->chunk,
                                ln->head->size);
  TEST_ASSERT_EQUAL(strlen("Hello, World!"), ln->head->size);

  free_node(ln->head);
  free(ln);
}

void test_line_node_delete_and_merge(void) {
  LineNode *ln1 = new_line(NULL);
  line_node_append(ln1, "Hello, ");

  LineNode *ln2 = new_line(ln1);
  ln1->next = ln2;
  ln2->prev = ln1;
  line_node_append(ln2, "World!");

  // delete "World!" from the start of the second line
  line_node_delete(ln2, 0, 6);

  TEST_ASSERT_NULL(ln1->next);
  TEST_ASSERT_EQUAL_STRING_SIZE("Hello, ", ln1->head->chunk, ln1->head->size);
  TEST_ASSERT_EQUAL(strlen("Hello, "), ln1->head->size);

  free_node(ln1->head);
  free(ln1);
}

#include "document.h"
Document *g_d = 0;

void test_document_init(void) {
  Document doc;
  document_init(&doc);

  TEST_ASSERT_NOT_NULL(doc.first_line);
  TEST_ASSERT_NOT_NULL(doc.last_line);
  TEST_ASSERT_EQUAL_PTR(doc.first_line, doc.last_line);
  TEST_ASSERT_EQUAL(1, doc.line_count);
  TEST_ASSERT_NULL(doc.line_index.index);
  TEST_ASSERT_EQUAL(0, doc.line_index.index_size);
  TEST_ASSERT_EQUAL(0, doc.line_index.line_gap);

  document_deinit(&doc);
}

void test_document_append(void) {
  Document doc;
  document_init(&doc);

  document_append(&doc, "Hello, World!");
  TEST_ASSERT_EQUAL_STRING("Hello, World!", doc.first_line->head->chunk);
  TEST_ASSERT_EQUAL(1, doc.line_count);

  document_deinit(&doc);
}

void test_document_newline(void) {
  Document doc;
  document_init(&doc);
  document_build_index(&doc, 2);

  document_append(&doc, "Hello, World!");
  document_newline(&doc);
  TEST_ASSERT_NOT_NULL(doc.first_line->next);
  TEST_ASSERT_EQUAL_PTR(doc.first_line->next, doc.last_line);
  TEST_ASSERT_EQUAL(2, doc.line_count);

  document_append(&doc, "Hello, World2!");
  document_newline(&doc);
  document_append(&doc, "Hello, World3!");
  // delete the `newline` (the 0th char) from the 2nd line and merge it with the
  // prev line
  document_delete_char(&doc, 1, 0);
  TEST_ASSERT_EQUAL(2, doc.line_count);

  // document_print_structure(&doc);

  document_deinit(&doc);
}

void test_document_build_index(void) {
  Document doc;
  document_init(&doc);

  for (int i = 0; i < 10; ++i) {
    char line[32];
    snprintf(line, sizeof(line), "Line %d", i + 1);
    document_append(&doc, line);
    document_newline(&doc);
  }

  document_build_index(&doc, 2);
  // document_print_structure(&doc);

  /*
  Index 0 -> LineNode starting with: "Line 1"
  Index 1 -> LineNode starting with: "Line 3"
  Index 2 -> LineNode starting with: "Line 5"
  Index 3 -> LineNode starting with: "Line 7"
  Index 4 -> LineNode starting with: "Line 9"
  Index 5 -> LineNode starting with: ""
  */

  TEST_ASSERT_EQUAL(6, doc.line_index.index_size);
  LineNode *ln = doc.line_index.index[0];
  TEST_ASSERT_EQUAL_STRING("Line 1", ln->head->chunk);

  ln = doc.line_index.index[2];
  TEST_ASSERT_EQUAL_STRING("Line 5", ln->head->chunk);

  document_deinit(&doc);
}

void test_document_find_line(void) {
  Document doc;
  document_init(&doc);

  for (int i = 0; i < 10; ++i) {
    char line[32];
    snprintf(line, sizeof(line), "Line %d", i + 1);
    document_append(&doc, line);
    document_newline(&doc);
  }

  document_build_index(&doc, 2);

  LineNode *ln = document_find_line(&doc, 4);
  TEST_ASSERT_NOT_NULL(ln);
  TEST_ASSERT_EQUAL_STRING("Line 5", ln->head->chunk);

  ln = document_find_line(&doc, 9);
  TEST_ASSERT_NOT_NULL(ln);
  TEST_ASSERT_EQUAL_STRING("Line 10", ln->head->chunk);

  document_deinit(&doc);
}

#include <time.h>

void test_document_benchmark(void) {
  Document doc;
  document_init(&doc);

  const char *test_string = "Test line of text!";
  int num_operations = 1000;
  int insert_count = 0;

  clock_t start_time = clock();

  // insertion of multiple lines
  for (int i = 0; i < num_operations; ++i) {
    // insert the string into the document and add a newline
    document_append(&doc, test_string);
    document_newline(&doc);
    insert_count++;
  }
  document_build_index(&doc, 2);

  // document_print_structure(&doc);
  TEST_ASSERT_EQUAL(num_operations + 1, doc.line_count);

  // deletion of alternate lines, in reverse order
  for (int i = num_operations / 2 - 1; i >= 0; --i) {
    document_delete_line(&doc, i * 2);
    document_build_index(&doc, 2);
    document_insert_newline(&doc, i * 2, 0);
    document_build_index(&doc, 2);
  }

  // insert characters into the middle of the document
  for (int i = 0; i < num_operations; ++i) {
    LineNode *ln = document_find_line(&doc, i % doc.line_count);
    if (ln) {
      insert_into_node(&ln->head, 5, "x");
    }
  }

  clock_t end_time = clock();
  double elapsed_time = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;

  printf("Benchmark completed in %.7f seconds\n", elapsed_time);
  printf("Total lines after operations: %ld\n", doc.line_count);

  document_deinit(&doc);
}

void setUp(void) { buffer_pool_init(POOL_SIZE); }

void tearDown(void) { buffer_pool_deinit(); }

typedef struct {
  const char *name;
  void (*test_func)(void);
} test_entry_t;

#define TEST_ENTRY(name)                                                       \
  { #name, name }

test_entry_t test_table[] = {
    TEST_ENTRY(test_buffer_pool_init),
    TEST_ENTRY(test_buffer_pool_alloc),
    TEST_ENTRY(test_buffer_pool_free),
    TEST_ENTRY(test_buffer_pool_realloc),
    TEST_ENTRY(test_create_node),
    TEST_ENTRY(test_merge_nodes),
    TEST_ENTRY(test_insert_into_node),
    TEST_ENTRY(test_delete_from_node),
    TEST_ENTRY(test_delete_from_node_wrong_index),
    TEST_ENTRY(test_delete_and_merge),
    TEST_ENTRY(test_complex_modifications),
    TEST_ENTRY(test_new_line),
    TEST_ENTRY(test_line_node_append),
    TEST_ENTRY(test_line_node_delete),
    TEST_ENTRY(test_line_node_replace),
    TEST_ENTRY(test_line_node_insert),
    TEST_ENTRY(test_line_node_delete_and_merge),
    TEST_ENTRY(test_document_init),
    TEST_ENTRY(test_document_append),
    TEST_ENTRY(test_document_newline),
    TEST_ENTRY(test_document_build_index),
    TEST_ENTRY(test_document_find_line),
    TEST_ENTRY(test_document_benchmark),
};

void run_all_tests(void) {
  for (size_t i = 0; i < sizeof(test_table) / sizeof(test_entry_t); i++) {
    UnityDefaultTestRun(test_table[i].test_func, test_table[i].name, 512);
  }
}

int main(int argc, char **argv) {
  set_log_level(INFO);
  UNITY_BEGIN();

  if (argc > 1) {
    const char *test_name = argv[1];
    for (size_t i = 0; i < sizeof(test_table) / sizeof(test_entry_t); i++) {
      if (strcmp(test_table[i].name, test_name) == 0) {
        RUN_TEST(test_table[i].test_func);
        return UNITY_END();
      }
    }
    printf("test '%s' not found.\n", test_name);
    return UNITY_END();
  }

  printf("running all tests...\n");
  run_all_tests();

  return UNITY_END();
}