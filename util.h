#ifndef EPOLL_UTIL_H
#define EPOLL_UTIL_H

void error(char *msg);
void log_pid(char *file);
void reap_zombies();
int listen_sock(char *port);
int make_non_blocking(int fd);

#endif
