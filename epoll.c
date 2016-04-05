#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <errno.h>
#include <sys/epoll.h>
#include <sys/wait.h>
#include <sys/socket.h>

#include "util.h"
#include "worker.h"
#include "parse.h"

#define MAX_CONNS 16
#define MAX_EVENTS 16
#define LOG_FILE "/tmp/netsh.pid"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Port number must be provided\n");
        return 0;
    }

    daemonize();
    log_pid(LOG_FILE);
    reap_zombies();

    int lsock, epollfd;
    struct epoll_event *events;

    lsock = listen_sock(argv[1]);
    if (make_non_blocking(lsock) == -1) 
        error("Failed to set non-blocking socket");
    if (listen(lsock, MAX_CONNS) == -1)
        error("Error on listen");
    if ((epollfd = epoll_create1(0)) == -1) 
        error("Failed to create epoll instance");

    struct epoll_event event_type;
    event_type.data.fd = lsock;
    event_type.events = EPOLLIN | EPOLLET;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, lsock, &event_type) == -1)
        error("Error on epoll_ctl");

    events = calloc(MAX_EVENTS, sizeof(event_type));
    // Event loop
    while (1) {
        int n, i, s;
        n = epoll_wait(epollfd, events, MAX_EVENTS, -1);
        for (i = 0; i < n; ++i) {
            if ((events[i].events & EPOLLERR) || 
                    (events[i].events & EPOLLHUP) || 
                    (!(events[i].events & EPOLLIN))) {
                perror("Epoll event error");
                close(events[i].data.fd);
                continue;
            } else if (lsock == events[i].data.fd) {
                while (1) {
                    struct sockaddr in_addr;
                    socklen_t in_len;
                    int connfd;

                    in_len = sizeof in_addr;
                    connfd = accept(lsock, &in_addr, &in_len);
                    if (connfd == -1) {
                        if ((errno != EAGAIN) && (errno != EWOULDBLOCK)) 
                            perror("accept");
                        break;
                    }
                    event_type.data.fd = connfd;
                    event_type.events = EPOLLIN | EPOLLET;
                    s = epoll_ctl(epollfd, EPOLL_CTL_ADD, connfd, &event_type);
                    if (s == -1) 
                        error("Error on epoll_ctl");
                }
                continue;
            } else {
                handle_sock(events[i].data.fd);
            }
        }
    }
    free(events);
    close(lsock);
    return EXIT_SUCCESS;
}

