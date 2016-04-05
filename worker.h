#ifndef EPOLL_WORKER_H
#define EPOLL_WORKER_H

typedef struct command {
    char *file;
    char **args;
} command;

int run(struct command** commands, size_t n, int outfd); 
int exec(command *cmd);

#endif
