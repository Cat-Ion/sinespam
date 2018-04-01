#include <errno.h>
#include <sys/select.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>

#define LISTEN_BACKLOG 16

void print_error(char const *str) {
  fprintf(stderr, "%s%s\n", str, strerror(errno));
}

int listen_on_port(char const * port) {
  struct addrinfo hints;
  struct addrinfo *result, *rp;
  int sfd, s;

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
  hints.ai_protocol = 0;          /* Any protocol */
  hints.ai_canonname = NULL;
  hints.ai_addr = NULL;
  hints.ai_next = NULL;

  s = getaddrinfo(NULL, port, &hints, &result);
  if(s != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
    return -1;
  }

  for (rp = result; rp != NULL; rp = rp->ai_next) {
    sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (sfd == -1) {
      continue;
    }
    if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0) {
      break;
    }
    close(sfd);
  }

  if (rp == NULL) {
    print_error("Could not bind: ");
    return -1;
  }

  freeaddrinfo(result);

  if (listen(sfd, LISTEN_BACKLOG) == -1) {
    print_error("Could not listen: ");
    return -1;
  }

  return sfd;
}

int accept_new_connection(int fd) {
  fd_set readfds;
  struct timeval timeout;
  FD_ZERO(&readfds);
  FD_SET(fd, &readfds);
  timeout.tv_sec = timeout.tv_usec = 0;
  select(fd + 1, &readfds, NULL, NULL, &timeout);

  if (FD_ISSET(fd, &readfds)) {
    return accept(fd, NULL, NULL);
  } else {
    return -1;
  }
}
