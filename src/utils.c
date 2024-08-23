#include "utils.h"

static LogLevel current_log_level = INFO;

void log_message(LogLevel level, const char *format, ...) {
  if (level > current_log_level) {
    return;
  }

  const char *level_str = "";
  switch (level) {
  case INFO:
    level_str = "[INFO] ";
    break;
  case WARNING:
    level_str = "[WARNING] ";
    break;
  case ERROR:
    level_str = "[ERROR] ";
    break;
  case DEBUG:
    level_str = "[DEBUG] ";
    break;
  }

  printf("%s", level_str);

  va_list args;
  va_start(args, format);
  vprintf(format, args);
  va_end(args);
}

void set_log_level(LogLevel level) { current_log_level = level; }
LogLevel get_current_log_level(void) { return current_log_level; }