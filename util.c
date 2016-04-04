#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <netinet/in.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/fcntl.h>

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

