#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include "synthesis.h"

#define DISCONNECT_TIMEOUT 10 //seconds

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
    process_forced_disconnects(); //lucas
    process_new_connections(); //lucas
    process_client_input();
    synthesize_sound();
  }
}

void process_forced_disconnects(){
	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC,&now);
	for(int i=0;i<num_clients;++i){
		struct client *c = &clients[i];
		if( now.tv_sec - c->last_read.tv_sec > DISCONNECT_TIMEOUT ){
			close(c->fd);
			num_clients--;
		}
	}
}
