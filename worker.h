#ifndef EPOLL_WORKER_H
#define EPOLL_WORKER_H

typedef struct command {
    char *file;
    char **args;
} command;

void run(command** commands, ssize_t n, int sockfd); 
void handle_conn(int fd);

#endif
