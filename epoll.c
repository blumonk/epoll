#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include <errno.h>
#include <sys/epoll.h>
#include <sys/wait.h>
#include <sys/socket.h>

#include "util.h"
#include "worker.h"

#define MAX_CONNS 16
#define MAX_EVENTS 16
#define BUF_SIZE 4096
#define LOG_FILE "/tmp/netsh.pid"

void test_handle(int fd) {
    if (fork() == 0) {
        printf("Started child!\n");
        int cnt = 0, ptr = 0;
        char buf[BUF_SIZE];
        while (1) {
            cnt = read(fd, buf + ptr, 512 - cnt); 
            /*if (cnt == -1) {*/
                /*printf("Finished with -1\n");*/
                /*break;*/
            /*}*/
            if (cnt == 0) {
                printf("Finished with 0\n");
                break;
            }
            ptr += cnt;
        }
        /*printf("%s\n", buf);*/
        int i = 0;
        close(fd);
        exit(0);
    } 
    close(fd);
}

int main(int argc, char *argv[]) {
    // Daemonize
    log_pid(LOG_FILE);
    reap_zombies();

    int lsock, epollfd;
    struct epoll_event *events;

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

    struct epoll_event event_type;
    event_type.data.fd = lsock;
    event_type.events = EPOLLIN | EPOLLET;
    /*event_type.events = EPOLLIN;*/
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
                    char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];

                    in_len = sizeof in_addr;
                    connfd = accept(lsock, &in_addr, &in_len);
                    if (connfd == -1) {
                        if ((errno != EAGAIN) && (errno != EWOULDBLOCK)) 
                            perror("accept");
                        break;
                    }

                    s = getnameinfo(&in_addr, in_len, hbuf, 
                            sizeof(hbuf), sbuf, sizeof(sbuf), 
                            NI_NUMERICHOST | NI_NUMERICSERV);

                    if (s == 0) printf("Hello, %s %s\n", hbuf, sbuf);

                    if (s == -1)
                        error("Failed to make connection socket non-blocking");
                    event_type.data.fd = connfd;
                    event_type.events = EPOLLIN | EPOLLET;
                    s = epoll_ctl(epollfd, EPOLL_CTL_ADD, connfd, &event_type);
                    if (s == -1) 
                        error("Error on epoll_ctl");
                }
                continue;
            } else {
                test_handle(events[i].data.fd);
            }
        }
    }
    free(events);
    close(lsock);
    return EXIT_SUCCESS;
}

