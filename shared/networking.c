#include <execinfo.h>
#include <signal.h>
#include <unistd.h>

#include "commands.h"
#include "networking.h"
#include "utils.h"

static network_cfg_t *global_network_cfg;
extern struct lfq_ctx *g_lfq_ctx;

bool command_logging = true;
FILE *command_log_path = 0;

#define STACK_SIZE 1024
void signal_handler(int signum) {
  char **backtrace_syms = {0};
  int backtrace_size = {0};

  if (signum == SIGINT || signum == SIGTERM || signum == SIGSEGV) {
    if (signum == SIGSEGV) {
      log_message(ERROR, "received segmentation fault, shutting down...");
      void *buffer[30];
      backtrace_size = backtrace(buffer, 30);
      backtrace_syms = backtrace_symbols(buffer, backtrace_size);
    } else {
      log_message(INFO, "received termination signal, shutting down...");
    }

    log_message(INFO, "sending termsig to tcl...");
    send_to_client("term");
    usleep(500);

    struct linger sl;
    sl.l_onoff = 1;  // enable linger option
    sl.l_linger = 0; // close immediately, without delay

    if (setsockopt(global_network_cfg->server_fd, SOL_SOCKET, SO_LINGER, &sl,
                   sizeof(sl)) < 0) {
      log_message(ERROR, "setsockopt(SO_LINGER) failed on server");
    }

    log_message(INFO, "closing server_fd...");
    if (global_network_cfg && global_network_cfg->server_fd > 0 &&
        close(global_network_cfg->server_fd) == 0) {
    } else {
      log_message(ERROR, "cant close server_fd");
    }

    if (setsockopt(global_network_cfg->client_fd, SOL_SOCKET, SO_LINGER, &sl,
                   sizeof(sl)) < 0) {
      log_message(ERROR, "setsockopt(SO_LINGER) failed on client");
    }

    log_message(INFO, "closing client_fd...");
    if (global_network_cfg && global_network_cfg->client_fd > 0 &&
        close(global_network_cfg->client_fd) == 0) {
    } else {
      log_message(INFO,
                  "cant close client_fd, but it should have closed already");
    }

    if (command_log_path) {
      fclose(command_log_path);
      log_message(INFO, "closing logfile...");
    }

    if (signum == SIGSEGV) {
      if (!backtrace_syms) {
        log_message(ERROR, "failed to capture stack trace");
      } else {
        fprintf(stderr, "resolving stack trace:\n");

        char binary_path[256];

        ssize_t len =
            readlink("/proc/self/exe", binary_path, sizeof(binary_path) - 1);
        if (len == -1) {
          perror("readlink failed");
          strncpy(binary_path, "./a.out", sizeof(binary_path));
        } else {
          binary_path[len] = '\0';
        }

        for (int i = 0; i < backtrace_size; i++) {
          char *start = strstr(backtrace_syms[i], "+");
          char *end = strstr(backtrace_syms[i], ")");
          if (start && end && start < end) {
            size_t len = end - start - 1;
            char offset[len + 1];
            strncpy(offset, start + 1, len);
            offset[len] = '\0';

            char command[256 * 2];
            snprintf(command, sizeof(command), "addr2line -e %s %s",
                     binary_path, offset);

            FILE *fp = popen(command, "r");
            if (fp == NULL) {
              log_message(ERROR, "popen failed for addr2line");
              continue;
            }

            char result[256];
            if (fgets(result, sizeof(result), fp) != NULL) {
              fprintf(stderr, "  %s \t\t%s", backtrace_syms[i], result);
            } else {
              fprintf(stderr, "%s: could not resolve\n", backtrace_syms[i]);
            }

            pclose(fp);
          } else {
            fprintf(stderr, "%s: malformed symbol entry\n", backtrace_syms[i]);
          }
        }
        free(backtrace_syms);
      }
    }
    log_message(INFO, "exiting...");
    exit(0);
  }
}

static void setup_signal_handling(network_cfg_t *n) {
  global_network_cfg = n;

  struct sigaction sa;
  sa.sa_handler = signal_handler;
  sa.sa_flags = 0;
  sigemptyset(&sa.sa_mask);

  if (sigaction(SIGINT, &sa, NULL) < 0) {
    log_message(ERROR, "sigaction for SIGINT failed");
    exit(1);
  }

  if (sigaction(SIGSEGV, &sa, NULL) < 0) {
    log_message(ERROR, "sigaction for SIGTERM failed");
    exit(1);
  }

  if (sigaction(SIGTERM, &sa, NULL) < 0) {
    log_message(ERROR, "sigaction for SIGSEGV failed");
    exit(1);
  }

  log_message(INFO, "signal handlers set up successfully");
}

static void set_nonblocking(int sockfd) {
  int flags = fcntl(sockfd, F_GETFL, 0);
  fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
}

static void init_networking(network_cfg_t *n) {
  n->client_fd = -1;

  if ((n->server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    log_message(ERROR, "creating socket failed");
    exit(1);
  }

  if (setsockopt(n->server_fd, SOL_SOCKET, SO_REUSEADDR, &n->opt,
                 sizeof(n->opt)) < 0) {
    log_message(ERROR, "setsockopt(SO_REUSEADDR) failed");
    close(n->server_fd);
    exit(1);
  }

  if (setsockopt(n->server_fd, SOL_SOCKET, SO_REUSEPORT, &n->opt,
                 sizeof(n->opt)) < 0) {
    log_message(ERROR, "setsockopt(SO_REUSEPORT) failed");
    close(n->server_fd);
    exit(1);
  }

  n->server_addr.sin_family = AF_INET;
  n->server_addr.sin_addr.s_addr = INADDR_ANY;
  n->server_addr.sin_port = htons(PORT);

  // bind socket to port
  if (bind(n->server_fd, (struct sockaddr *)&n->server_addr,
           sizeof(n->server_addr)) < 0) {
    close(n->server_fd);
    log_message(ERROR,
                "bind failed on %d,"
                " probably in TIME_WAIT check with 'netstat -an | grep :%d' "
                "and wait some",
                PORT, PORT);
    exit(1);
  }

  // incoming connections
  if (listen(n->server_fd, 3) < 0) {
    log_message(ERROR, "listen failed");
    exit(1);
  }

  log_message(INFO, "c server listening on port %d...", PORT);

  // socket to non-blocking mode
  set_nonblocking(n->server_fd);
}

static void accept_data(network_cfg_t *n) {

  // clear the set of read file descriptors
  FD_ZERO(&n->readfds);

  // add the server socket to the set
  FD_SET(n->server_fd, &n->readfds);
  n->max_sd = n->server_fd;

  // add any existing client socket to the set
  if (n->client_fd > 0) {
    FD_SET(n->client_fd, &n->readfds);
    if (n->client_fd > n->max_sd) {
      n->max_sd = n->client_fd;
    }
  }

  // set timeout to 1 second
  struct timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec = 50000; // 10 milliseconds

  // wait for an activity on one of the sockets
  int activity = select(n->max_sd + 1, &n->readfds, NULL, NULL, &timeout);

  if (activity < 0 && errno != EINTR) {
    log_message(ERROR, "select error");
    exit(1);
  }

  // incoming connection
  if (FD_ISSET(n->server_fd, &n->readfds)) {
    n->client_len = sizeof(n->client_addr);
    if ((n->client_fd = accept(n->server_fd, (struct sockaddr *)&n->client_addr,
                               &n->client_len)) < 0) {
      log_message(ERROR, "accept failed");
    } else {
      log_message(INFO, "client connected!");
      set_nonblocking(
          n->client_fd); // make the client socket non-blocking as well
      if (n->startup_cmd) {
        send_to_client(n->startup_cmd);
      }
    }
  }
}

static void log_command(char *command) {
  if (command_logging && command_log_path) {
    fprintf(command_log_path, "%s\n", command);
    fflush(command_log_path);
  }
}

static void parse_and_ex_command(network_cfg_t *n) {
  char head[50] = {0};
  if (sscanf(n->buffer, "%s", head) != 1) {
    log_message(ERROR, "invalid command format: ->%s<-", n->buffer);
    send_to_client("cmdnack");
    return;
  }
  char *ptr = n->buffer;
  sscanf(ptr, "%s", head); // skip command
  ptr += strlen(head);
  while (*ptr == ' ')
    ptr++;

  arg_t args[MAX_ARGS] = {0};
  int i = 0;
  while (i < MAX_ARGS && sscanf(ptr, "%s", args[i].data) == 1) {
    ptr += strlen(args[i].data);
    // log_message(DEBUG, "arg: ->%s<-", args[i].data);
    while (*ptr == ' ')
      ptr++;
    i++;
  }

  command_fn f = find_function_by_command(head, i);
  if (f) {
    f(args, i);
    // only log valid commands tho, in terms of repro the others dont matter
    log_command(n->buffer);
  } else {
    log_message(ERROR, "invalid command: `%s`", head);
    send_to_client("cmdnack");
  }
}

static void handle_data(network_cfg_t *n) {
  // check if the client has sent data
  if (n->client_fd > 0 && FD_ISSET(n->client_fd, &n->readfds)) {
    memset(n->buffer, 0, MSG_BUFFER_SIZE);
    int valread = read(n->client_fd, n->buffer, MSG_BUFFER_SIZE);

    if (valread > 0) {
      size_t len = strlen(n->buffer);
      if (len > 0 &&
          (n->buffer[len - 1] == '\n' || n->buffer[len - 1] == '\r')) {
        n->buffer[len - 1] = '\0';
      }
      log_message(DEBUG, "message from tcl: `%s`", n->buffer);
      parse_and_ex_command(n);

    } else if (valread == 0) {
      // client has disconnected
      log_message(INFO, "client disconnected.");
      close(n->client_fd);
      n->client_fd = -1; // reset the client_fd
    }
  }
}

static void send_data(network_cfg_t *n, const char *data) {
  if (n->client_fd > 0) {
    ssize_t bytes_sent = send(n->client_fd, data, strlen(data), 0);

    if (bytes_sent < 0) {
      log_message(ERROR, "error sending");
      close(n->client_fd);
      n->client_fd = -1;
    } else {
      log_message(DEBUG, "sent %ld bytes", bytes_sent);
    }
  }
}

void send_to_client(const char *format, ...) {
  if (!global_network_cfg) {
    return;
  }

  char buffer[1024];

  va_list args;
  va_start(args, format);

  vsnprintf(buffer, sizeof(buffer), format, args);

  va_end(args);
  // still need this to be generic
  if (buffer[strlen(buffer) - 1] != '\n') {
    strcat(buffer, "\n");
  }

  send_data(global_network_cfg, buffer);
}

void networking_thread(network_cfg_t *n) {
  setup_signal_handling(n);
  init_networking(n);

  while (1) {
    accept_data(n);
    handle_data(n);
  }

  close(n->server_fd);
}