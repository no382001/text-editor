#include "commands.h"
#include "utils.h"
#include <stdatomic.h>

// refactor this so it can handle multiple arguments if needed
static command_map_t cmd_map[] = {
    {"set", key_pressed}, {"res", key_released}, {NULL, NULL}};

void key_released(char *userdata) {}
void set_key_pressed(char key, bool pressed) {
}

void key_pressed(char *userdata) {
  if (strlen(userdata) != 1) {
    log_message(ERROR, "invalid command tail: %s", userdata);
    return;
  }
  
  log_message(ERROR, "got some shit: %s", userdata);
}

command_fn find_function_by_command(const char *command) {
  for (int i = 0; cmd_map[i].command != NULL; i++) {
    if (strcmp(cmd_map[i].command, command) == 0) {
      return cmd_map[i].f;
    }
  }
  return NULL;
}