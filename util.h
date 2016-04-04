#ifndef EPOLL_UTIL_H
#define EPOLL_UTIL_H

void error(char *msg);
int listen_sock(char *port);
int make_non_blocking(int fd);

#endif
