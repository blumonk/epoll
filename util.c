#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <netinet/in.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/fcntl.h>
#include <sys/wait.h>
#include <errno.h>

void error(char *msg) {
    perror(msg);
    exit(1);
}

int listen_sock(char *port) {
    int lsock, portno;
    struct sockaddr_in addr;
    lsock = socket(AF_INET, SOCK_STREAM, 0);
    if (lsock < 0) 
        error("Failed to open welcoming TCP socket");
    bzero((char *) &addr, sizeof(addr));
    portno = atoi(port);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(portno);
    if (bind(lsock, (struct sockaddr *) &addr, sizeof(addr)) < 0) 
        error("Failed to bind socket");
    return lsock;
}

int make_non_blocking(int fd) {
    int flags, s;
    flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
        return -1;
    flags |= O_NONBLOCK;
    s = fcntl(fd, F_SETFL, flags);
    if (s == -1)
        return -1;
    return 0;
}

void log_pid(char *file) {
    FILE *f = fopen(file, "w");
    if (f == NULL) {
        error("Failed to open log file");
    }
    fprintf(f, "%d", getpid());
    fclose(f);
}

void handle_sigchld(int sig) {
    int saved_errno = errno;
    while (waitpid((pid_t)(-1), 0, WNOHANG) > 0) {}
    errno = saved_errno;
}

void reap_zombies() {
    struct sigaction sa;
    sa.sa_handler = &handle_sigchld;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    if (sigaction(SIGCHLD, &sa, 0) == -1) {
        perror(0);
        exit(1);
    }
}
