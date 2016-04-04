#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/socket.h>

#include "util.h"

#define MAX_CONNS 8

int main(int argc, char *argv[]) {
    int lsock, epollfd;
    struct epoll_event event;

    if (argc < 2) {
        fprintf(stderr, "Port number must be provided\n");
        return 0;
    }

    lsock = listen_sock(argv[1]);
    if (make_non_blocking(lsock) == -1) 
        error("Failed to set non-blocking socket");
    if (listen(lsock, MAX_CONNS) == -1)
        error("Error on listen");
    if ((epollfd = epoll_create1(0)) == -1) 
        error("Failed to create epoll instance");

    event.data.fd = lsock;
    event.events = EPOLLIN | EPOLLET;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, lsock, &event) == -1)
        error("Error on epoll_ctl");

    return EXIT_SUCCESS;
}

