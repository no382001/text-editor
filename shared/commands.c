#include "commands.h"
#include "document.h"
#include "networking.h"
#include "utils.h"
#include <stdatomic.h>

#include <ctype.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/evp.h>
#include <time.h>

key_translation_t key_translation_table[] = {
    {"space", " "},       {"BackSpace", "BackSpace"},
    {"Tab", "\t"},        {"Return", "Return"},
    {"F5", "F5"},

    {"exclam", "!"},      {"at", "@"},
    {"numbersign", "#"},  {"dollar", "$"},
    {"percent", "%"},     {"asciicircum", "^"},
    {"ampersand", "&"},   {"asterisk", "*"},
    {"parenleft", "("},   {"parenright", ")"},
    {"minus", "-"},       {"underscore", "_"},
    {"plus", "+"},        {"equal", "="},
    {"bracketleft", "["}, {"bracketright", "]"},
    {"braceleft", "{"},   {"braceright", "}"},
    {"semicolon", ";"},   {"colon", ":"},
    {"quote", "'"},       {"doublequote", "\""},
    {"backslash", "\\"},  {"slash", "/"},
    {"comma", ","},       {"period", "."},
    {"less", "<"},        {"greater", ">"},
    {"question", "?"},    {"bar", "|"},
    {"grave", "`"},       {"asciitilde", "~"},
    {NULL, NULL}};

const char *translate_key(const char *key) {
  for (int i = 0; key_translation_table[i].name != NULL; i++) {
    if (strcmp(key, key_translation_table[i].name) == 0) {
      return key_translation_table[i].repl;
    }
  }
  return "";
}

char *base64_encode(char *buffer, size_t length) {
  BIO *bio = NULL, *b64 = NULL;
  BUF_MEM *bufferPtr = NULL;
  char *b64text = NULL;

  if (length <= 0)
    goto cleanup;

  b64 = BIO_new(BIO_f_base64());
  if (b64 == NULL)
    goto cleanup;

  bio = BIO_new(BIO_s_mem());
  if (bio == NULL)
    goto cleanup;

  bio = BIO_push(b64, bio);

  if (BIO_write(bio, buffer, (int)length) <= 0)
    goto cleanup;

  if (BIO_flush(bio) != 1)
    goto cleanup;

  BIO_get_mem_ptr(bio, &bufferPtr);

  b64text = (char *)malloc((bufferPtr->length + 1) * sizeof(char));
  if (b64text == NULL)
    goto cleanup;

  memcpy(b64text, bufferPtr->data, bufferPtr->length);
  b64text[bufferPtr->length] = '\0';
  BIO_set_close(bio, BIO_NOCLOSE);

cleanup:
  BIO_free_all(bio);
  return b64text;
}

extern Document *g_d;

#define EDITOR_LINES "14"
// - update viewport
//  - viewport <start_line> <end_line>
void viewport(arg_t *args, int size) {

  int from = atoi(args[0].data);
  int to = atoi(args[1].data);

  for (int i = from; i < to; i++) {

    LineNode *ln = document_find_line(g_d, i);

    char buf[MSG_BUFFER_SIZE] = {0};
    print_node_to_buffer(ln->head, buf, MSG_BUFFER_SIZE);
    if (strlen(buf) == 0) {
      send_to_client("el %d", i);
    } else {
      char *b64uf = base64_encode(buf, strlen(buf));
      send_to_client("ch %d %s", i, b64uf);
    }
  }
  // send_to_client("pos %d %d", 0, 0);
}

// refactor this so it can handle multiple arguments if needed
static command_map_t cmd_map[] = {
    {"key", key_pressed, 3}, {"viewport", viewport, 2}, {NULL, NULL}};

// - insert/delete into/from document
//   - key <col> <row> <button>
//   - key <col> <row> BackSpace
void key_pressed(arg_t *args, int size) {

  int line = atoi(args[0].data);
  int col = atoi(args[1].data);
  const char *key = args[2].data;

  log_message(DEBUG, "key_pressed %d %d %s", line, col, key);

  if (strlen(key) != 1) {
    key = translate_key(key);
    if (strlen(key) == 0) { // not in the table, its whatever
      return;
    }
  }

  LineNode *ln = document_find_line(g_d, line);
  print_line_node(ln);
  printf(" -> ");

  if (!strcmp(key, "F5")) {
    viewport((arg_t[]){{"0"}, {EDITOR_LINES}}, 2);

  } else if (!strcmp(key, "BackSpace")) {
    delete_from_node(&ln->head, col, 1);
    print_line_node(ln);
    send_to_client("pos %d %d", line, col > 0 ? col - 1 : 0);

  } else if (!strcmp(key, "Return")) {
    line_node_insert_newline(ln, col);
    viewport((arg_t[]){{"0"}, {EDITOR_LINES}}, 2);
    send_to_client("pos %d %d", line + 1, 0);

  } else {
    insert_into_node(&ln->head, col, key);
    print_line_node(ln);
    send_to_client("pos %d %d", line, col + 1);
  }
  printf("\n\n");

  char buf[MSG_BUFFER_SIZE] = {0};
  print_node_to_buffer(ln->head, buf, MSG_BUFFER_SIZE);

  // there might not have been any change in edge cases
  // so the command is redundant

  if (strlen(buf) == 0) {
    send_to_client("el %d", line);
  } else {
    char *b64uf = base64_encode(buf, strlen(buf));
    send_to_client("ch %d %s", line, b64uf);
  }
}

command_fn find_function_by_command(const char *command, int arity) {
  for (int i = 0; cmd_map[i].command != NULL; i++) {
    if (strcmp(cmd_map[i].command, command) == 0 && arity == cmd_map[i].arity) {
      return cmd_map[i].f;
    }
  }
  return NULL;
}