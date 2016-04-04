#include "util.h"

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

