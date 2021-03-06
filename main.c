#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include "network.h"
#include "parser.h"
#include "synthesis.h"

#define DISCONNECT_TIMEOUT 10 //seconds
#define BUFFER_SIZE 128
#define MAX_CLIENTS 64

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
} clients[MAX_CLIENTS];

void force_disconnect_client(int i, bool update_max_fd){
  struct client *c = &clients[i];
  close(c->fd);
  num_clients--;
  memmove(&clients[i],&clients[i+1],sizeof(struct client)*(num_clients-i));
  if( update_max_fd ){
    max_fd = 0;
    for(int k=0;k<num_clients;++k){
      int myfd = clients[k].fd;
      if( myfd > max_fd ){
        max_fd = myfd;
      }
    }
  }
}

void process_forced_disconnects(){
  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC,&now);
  int my_max_fd=0;
  for(int i=0;i<num_clients;++i){
    struct client *c = &clients[i];
    bool read_timeout = ( now.tv_sec - c->last_read.tv_sec ) > DISCONNECT_TIMEOUT;
    bool write_timeout = ( now.tv_sec - c->last_write.tv_sec ) > DISCONNECT_TIMEOUT;
    if( read_timeout || write_timeout ){
      force_disconnect_client(i,false);
      i--;
    }else{
      if(my_max_fd < c->fd){
        my_max_fd = c->fd;
      }
    }
  }
  max_fd = my_max_fd;
}

void process_new_connections(){
  if( num_clients >= MAX_CLIENTS ){
    return;
  }
  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC,&now);
  int fd;
  while( (fd = accept_new_connection(listen_fd)) != -1 ) {
    struct client *c = &clients[num_clients];
    fprintf(stderr, "New client connection on fd %d\n", fd);
    c->fd = fd;
    c->last_read = now;
    c->last_write = now;
    c->in_length = 0;
    c->out_length = 0;
    if( fd > max_fd ){
      max_fd = fd;
    }
    num_clients++;
    if( num_clients >= MAX_CLIENTS ){
      return;
    }
  }
}

void read_from_client(int client_index) {
  struct timespec now;
  struct client *c = &clients[client_index];
  int recv_len = recv(c->fd, c->in_buffer + c->in_length, BUFFER_SIZE - c->in_length, MSG_DONTWAIT);
  clock_gettime(CLOCK_MONOTONIC, &now);
  fprintf(stderr, "Read %d bytes\n", recv_len);
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
  } else {
    force_disconnect_client(client_index, true);
  }
}

void process_client_input(void) {
  struct timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec = 10000;
  
  for(size_t ci = 0; ci < num_clients; ci++) {
    FD_SET(clients[ci].fd, &read_fds);
    FD_SET(clients[ci].fd, &write_fds);
  }

  select(max_fd + 1, &read_fds, &write_fds, 0, &timeout);

  for(size_t ci = 0; ci < num_clients; ci++) {
    struct client *c = &clients[ci];
    if (FD_ISSET(c->fd, &read_fds)) {
      fprintf(stderr, "Can read from client %d\n", ci);
      read_from_client(ci);
    }
  }
}

struct timespec last_sound_generation;
void synthesize_sound(void) {
  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);

  double dt = (now.tv_sec - last_sound_generation.tv_sec)
            + (now.tv_nsec - last_sound_generation.tv_nsec) * 1e-9;

  uint32_t samples = (uint32_t)(dt * SAMPLING_RATE);
  uint32_t seconds = samples / SAMPLING_RATE;
  uint32_t nanoseconds = (samples % SAMPLING_RATE) * 1000000000 / SAMPLING_RATE;

  now.tv_sec = last_sound_generation.tv_sec + seconds + (last_sound_generation.tv_nsec + nanoseconds) / 1000000000;
  now.tv_nsec = (last_sound_generation.tv_nsec + nanoseconds) % 1000000000;

  fprintf(stderr, "Sound gen: %3d.%09d - %3d.%09d\n", last_sound_generation.tv_sec % 1000, last_sound_generation.tv_nsec, now.tv_sec % 1000, now.tv_nsec);

  int32_t buf[128];
  fprintf(stderr, "Generating %d samples\n", samples);
  while (samples > 0) {
    uint32_t iteration_samples = 128;
    if (iteration_samples > samples) {
      iteration_samples = samples;
    }
    synthesis_generate_sound(&buf[0], iteration_samples);
    write(1, &buf[0], iteration_samples * sizeof(uint32_t));
    samples -= iteration_samples;
  }
  
  last_sound_generation.tv_sec = now.tv_sec;
  last_sound_generation.tv_nsec = now.tv_nsec;
}

int main() {
  synthesis_init();
  clock_gettime(CLOCK_MONOTONIC, &last_sound_generation);

  listen_fd = listen_on_port("1235");
  fprintf(stderr, "Listen FD: %d\n", listen_fd);
  for (;;) {
    process_forced_disconnects(); //lucas done
    process_new_connections(); //lucas done
    process_client_input();
    synthesize_sound();
  }
}
