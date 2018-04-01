
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include "network.h"
#include "parser.h"
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

void read_from_client(int client_index) {
  struct timespec now;
  struct client *c = &clients[client_index];
  int recv_len = recv(c->fd, c->in_buffer + c->in_length, BUFFER_SIZE - c->in_length, MSG_DONTWAIT);
  clock_gettime(CLOCK_MONOTONIC, &now);
  if (recv_len > 0) {
    int start = c->in_length;
    c->in_length += recv_len;
    c->last_read = now;
    for (int i = start; i < c->in_length; i++) {
      if (c->in_buffer[i] == '\n') {
        char *answer = NULL;
        size_t answer_length = 0;
        c->in_buffer[i] = '\0';
        parse(c->in_buffer, i, &answer, &answer_length);
        if (answer_length + c->out_length > BUFFER_SIZE) {
          force_disconnect_client(client_index, true);
        } else {
          memcpy(&c->out_buffer[c->out_length], answer, answer_length);
          c->out_length += answer_length;
          free(answer);
        }
        c->in_length -= i;
        i = -1;
      }
    }
  }
}
void process_client_input(void) {
  struct timeval timeout;
  timeout.tv_sec = timeout.tv_usec = 0;
  select(max_fd + 1, &read_fds, &write_fds, 0, &timeout);

  for(size_t ci = 0; ci < num_clients; ci++) {
    struct client *c = &clients[ci];
    if (FD_ISSET(c->fd, &read_fds)) {
      read_from_client(ci);
    }
  }
}
void synthesize_sound(void) {
}
int main() {
  synthesis_init();

  listen_fd = listen_on_port("1235");
  for (;;) {
    process_forced_disconnects();
    process_new_connections();
    process_client_input();
    synthesize_sound();
  }
}
