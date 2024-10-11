#include "commands.h"
#include "document.h"
#include "networking.h"
#include "utils.h"
#include <stdatomic.h>

#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>
#include <time.h>

char *base64_encode(char *buffer, size_t length)
{
    BIO *bio = NULL, *b64 = NULL;
    BUF_MEM *bufferPtr = NULL;
    char *b64text = NULL;

    if(length <= 0) goto cleanup;

    b64 = BIO_new(BIO_f_base64());
    if(b64 == NULL) goto cleanup;

    bio = BIO_new(BIO_s_mem());
    if(bio == NULL) goto cleanup;

    bio = BIO_push(b64, bio);

    if(BIO_write(bio, buffer, (int)length) <= 0) goto cleanup;

    if(BIO_flush(bio) != 1) goto cleanup;

    BIO_get_mem_ptr(bio, &bufferPtr);

    b64text = (char*) malloc((bufferPtr->length + 1) * sizeof(char));
    if(b64text == NULL) goto cleanup;

    memcpy(b64text, bufferPtr->data, bufferPtr->length);
    b64text[bufferPtr->length] = '\0';
    BIO_set_close(bio, BIO_NOCLOSE);

cleanup:
    BIO_free_all(bio);
    return b64text;
}

extern Document *g_d;

// - update viewport
//  - viewport <start_line> <end_line>
void viewport(arg_t *args, int size){
  if (size != 2) {
    log_message(ERROR, "got wrong size: %d instead of %d", size, 2);
    return;
  }

  int from = atoi(args[0].data);
  int to = atoi(args[1].data);

  for(int i = from; i < to; i++){

    LineNode *ln = document_find_line(g_d, i);

    char buf[MSG_BUFFER_SIZE] = {0};
    print_node_to_buffer(ln->head,buf,MSG_BUFFER_SIZE);
    char *b64uf = base64_encode(buf, strlen(buf));
    
    send_to_client("ch %d %d %s", i, 0, b64uf);
  }
  send_to_client("pos %d %d", 0, 0);
}

// refactor this so it can handle multiple arguments if needed
static command_map_t cmd_map[] = {{"key", key_pressed},{"viewport", viewport},{NULL, NULL}};

// - insert/delete into/from document
//   - key <col> <row> <button>
//   - key <col> <row> BackSpace
void key_pressed(arg_t *args, int size) {
  if (size != 3) {
    log_message(ERROR, "got wrong size: %d instead of %d", size, 3);
    return;
  }

  int line = atoi(args[0].data);
  int col = atoi(args[1].data);
  char *key = args[2].data;

  LineNode *ln = document_find_line(g_d, line);
  print_line_node(ln); printf(" -> ");

  if (strcmp(key,"space") == 0){
    key = " ";
  }

  if (strcmp(key, "BackSpace") == 0) {
    delete_from_node(&ln->head, col, 1);
    print_line_node(ln);
    send_to_client("pos %d %d", line, col - 1);
  } else {
    insert_into_node(&ln->head,col,key);
    print_line_node(ln); 
    send_to_client("pos %d %d", line, col + 1);
  }
  putc('\n',stdout);
  fflush(stdout);

  // fugen print it to a buffer, idk
  char buf[MSG_BUFFER_SIZE] = {0};
  print_node_to_buffer(ln->head,buf,MSG_BUFFER_SIZE);
  char *b64uf = base64_encode(buf, strlen(buf));
  send_to_client("ch %d %d %s", line, col, b64uf);
  
  //send_to_client("pos %d %d", line, col - 1);
}

command_fn find_function_by_command(const char *command) {
  for (int i = 0; cmd_map[i].command != NULL; i++) {
    if (strcmp(cmd_map[i].command, command) == 0) {
      return cmd_map[i].f;
    }
  }
  return NULL;
}