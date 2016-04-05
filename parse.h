#ifndef EPOLL_PARSE_H
#define EPOLL_PARSE_H

#include "worker.h"

command** parse(char *buffer, ssize_t size);
command* parse_one(char *buffer, ssize_t from, ssize_t to);
ssize_t carr_ret(char *buffer, ssize_t size);
ssize_t cmd_cnt(char *buffer, ssize_t size);

#endif
