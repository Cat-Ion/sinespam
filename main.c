#include <stdint.h>
#include <stdio.h>
#include "synthesis.h"

int listen_fd;
int num_clients;
struct client {
  int fd;
  size_t in_length, out_length;
  struct timespec last_read, last_write;
  char in_buffer[80];
  char out_buffer[80];
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
