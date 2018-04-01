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
	int fd = accept_new_connection(listen_fd);
	while( fd != -1 ){
		struct client *c = &clients[num_clients];
		c->fd = fd;
		c->last_read = now;
		c->last_write = now;
		if( fd > max_fd ){
			max_fd = fd;
		}
		num_clients++;
		if( num_clients >= MAX_CLIENTS ){
			return;
		}
		fd = accept_new_connection(listen_fd);
	}
}

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
    process_forced_disconnects(); //lucas done
    process_new_connections(); //lucas done
    process_client_input();
    synthesize_sound();
  }
}
