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
#include <math.h>
int base64_encode(const char *message, char *buffer, size_t buffer_size) {
  BIO *bio, *b64;
  FILE *stream;

  int message_len = strlen(message);
  int encodedSize = 4 * ceil((double)message_len / 3);

  if (buffer_size < encodedSize + 1) {
    return -1;
  }

  stream = fmemopen(buffer, buffer_size, "w");
  if (!stream) {
    return -1;
  }

  b64 = BIO_new(BIO_f_base64());
  bio = BIO_new_fp(stream, BIO_NOCLOSE);
  bio = BIO_push(b64, bio);
  BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);

  BIO_write(bio, message, message_len);
  BIO_flush(bio);

  BIO_free_all(bio);
  fclose(stream);

  return 0;
}

extern Document *g_d;

#define EDITOR_LINES "14"

// i need to somehow catch the last viewport and keep sending that back on
// modifications

// - update viewport
//  - viewport <start_line> <end_line>
void viewport(arg_t *args, int size) {

  // maybe a bit confusing
  int from = atoi(args[0].data);
  int to = atoi(args[1].data);
  int offset = atoi(args[2].data); // default is 0 if it cant convert

  for (int i = from; i < from + to; i++) {

    LineNode *ln = document_find_line(g_d, i + offset);

    char buf[MSG_BUFFER_SIZE] = {0};
    print_node_to_buffer(ln->head, buf, MSG_BUFFER_SIZE);
    if (strlen(buf) == 0) {
      send_to_client("el %d", i);
    } else {
      char encode[MSG_BUFFER_SIZE] = {0};
      base64_encode(buf, encode, MSG_BUFFER_SIZE);
      send_to_client("ch %d %s", i, encode);
    }
  }
  // send_to_client("pos %d %d", 0, 0);
}

static command_map_t cmd_map[] = {
    {"key", key_pressed, 3},
    {"viewport", viewport, 2}, // this is fundamentally wrong
    {"viewport", viewport, 3},
    {NULL, NULL}};

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

  if (!strcmp(key, "F5")) {
    viewport((arg_t[]){{"0"}, {EDITOR_LINES}, {""}}, 2);
    return;

  } else if (!strcmp(key, "BackSpace")) {
    document_delete_char(g_d, line, col);
    document_build_index(g_d, DOCUMENT_INDEX_GAP);
    send_to_client("move left");
    if (col == 0) {
      viewport((arg_t[]){{"0"}, {EDITOR_LINES}, {""}}, 2);
    }
    ln = document_find_line(g_d, line);

  } else if (!strcmp(key, "Return")) {
    document_insert_newline(g_d, line, col);
    document_build_index(g_d, DOCUMENT_INDEX_GAP);
    viewport((arg_t[]){{"0"}, {EDITOR_LINES}, {""}}, 2);
    send_to_client("move right");
    return;

  } else {
    insert_into_node(&ln->head, col, key);
  }

  char buf[MSG_BUFFER_SIZE] = {0};
  chk_ptr(ln->head);
  print_node_to_buffer(ln->head, buf, MSG_BUFFER_SIZE);
  // there might not have been any change in edge cases
  // so the command is redundant

  if (strlen(buf) == 0) {
    send_to_client("el %d", line);
  } else {
    char encode[MSG_BUFFER_SIZE] = {0};
    base64_encode(buf, encode, MSG_BUFFER_SIZE);
    send_to_client("ch %d %s", line, encode);
  }

  if (strcmp(key, "BackSpace")) {
    send_to_client("move right");
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