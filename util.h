#ifndef EPOLL_UTIL_H
#define EPOLL_UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <netinet/in.h>
#include <sys/types.h> 
#include <sys/socket.h>

void error(char *msg);
int listen_sock(char *port);

#endif
