#pragma once
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

typedef enum {
  ERROR,
  INFO,
  WARNING,
  DEBUG,
} LogLevel;

typedef struct {
  LogLevel level;
  const char *file;
  int line;
  char message[1024];
  int repeat;
} LogState;

void log_message_impl(LogLevel level, const char *file, int line,
                      const char *format, ...);
void log_message_impl2(const char *level_str, const char *format, ...);
void set_log_level(LogLevel level);
LogLevel get_current_log_level();

// wtf does cmake c11 use? this works different there
// #define __RELATIVE_FILE__ (__FILE__ + SOURCE_PATH_SIZE)

#define __RELATIVE_FILE__ (__FILE__)

#define log_and_execute(level, message, func)                                  \
  do {                                                                         \
    log_message_impl(level, message);                                          \
    if (level <= get_current_log_level()) {                                    \
      func;                                                                    \
    }                                                                          \
  } while (0)

#define LOGGING

#ifdef LOGGING
#define log_message(level, message, ...)                                       \
  log_message_impl(level, __RELATIVE_FILE__, __LINE__, message, ##__VA_ARGS__)
#else
#define log_message(level, message, ...)
#endif
//
/*
static void print_path() {
  char cwd[PATH_MAX];

  if (getcwd(cwd, sizeof(cwd)) != NULL) {
    printf("Current working directory: %s\n", cwd);
  } else {
    perror("getcwd() error");
  }
}
*/
#include <signal.h>
void signal_handler(int signum);
#define chk_ptr(p)                                                             \
  do {                                                                         \
    if (!p) {                                                                  \
      log_message(ERROR, "chk_ptr failed");                                    \
      signal(SIGTERM,signal_handler);                                          \
    }                                                                          \
  } while (0)

//