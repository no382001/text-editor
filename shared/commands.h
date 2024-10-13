#pragma once
#include <stdbool.h>

#define MAX_ARGS 10
#define MAX_ARGS_SIZE 128

typedef struct arg_t {
  char data[MAX_ARGS_SIZE];
} arg_t;

typedef void (*command_fn)(arg_t *, int size);

typedef struct command_map {
  char *command;
  command_fn f;
  int arity;
} command_map_t;

typedef struct {
  const char *name;
  const char *repl;
} key_translation_t;

void key_pressed(arg_t *, int size);
void key_released(char *userdata);
command_fn find_function_by_command(const char *command, int arity);