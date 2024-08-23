#pragma once
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

typedef enum {
  INFO,
  WARNING,
  ERROR,
  DEBUG,
} LogLevel;

void log_message(LogLevel level, const char *format, ...);
void set_log_level(LogLevel level);
LogLevel get_current_log_level();

#define log_and_execute(level, message, func)                                  \
  do {                                                                         \
    log_message(level, message);                                               \
    if (level <= get_current_log_level()) {                                    \
      func;                                                                    \
    }                                                                          \
  } while (0)