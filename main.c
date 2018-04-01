#include <stdint.h>
#include <stdio.h>
#include <sys/select.h>
#include "synthesis.h"

#define BUFFER_SIZE 128

int listen_fd;
int max_fd;
int num_clients;
fd_set read_fds, write_fds;

struct client {
  int fd;
  size_t in_length, out_length;
  struct timespec last_read, last_write;
  char in_buffer[BUFFER_SIZE];
  char out_buffer[BUFFER_SIZE];
} clients[64];

int main() {
  int32_t buf[128];
  synthesis_init();

  int listenfd = listen_on_port(1235);
  for (;;) {
    process_forced_disconnects();
    process_new_connections();
    process_client_input();
    synthesize_sound();
  }
}
