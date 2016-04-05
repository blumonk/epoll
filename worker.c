#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>

#include "worker.h"
#include "parse.h"

#define BUF_SIZE 4096

int exec(command *cmd) {
    pid_t pid = fork();
    if (pid == -1) {
        return -1;
    } else if (pid == 0) {
        execvp(cmd->file, cmd->args);
    }
    return pid;
}

#define SAFE(val) \
    if (val == -1) exit(1); 

void run(command** commands, ssize_t n, int sockfd) {
    int pipes[2 * n];
    int pids[n];
    SAFE(dup2(sockfd, STDIN_FILENO));
    for (int i = 0; i < n - 1; i++) {
        SAFE(pipe(pipes + 2 * i));
        int r = pipes[i * 2];
        int w = pipes[i * 2 + 1];
        SAFE(fcntl(w, F_SETFD, FD_CLOEXEC));
        SAFE(dup2(w, STDOUT_FILENO));
        pids[i] = exec(commands[i]);
        SAFE(pids[i]);
        SAFE(dup2(r, STDIN_FILENO));
    }
    SAFE(dup2(sockfd, STDOUT_FILENO));
    SAFE(dup2(sockfd, STDERR_FILENO));
    pids[n - 1] = exec(commands[n - 1]);
    SAFE(pids[n - 1]);
}

void handle_sock(int sockfd) {
    if (fork() == 0) {
        ssize_t total = 0, cnt = 0;
        char buf[BUF_SIZE];
        while (1) {
            cnt = read(sockfd, buf + total, 1); 
            if (cnt == 0) 
                break;
            total += cnt;
            if (buf[total - 1] == '\n')
                break;
        }
        command **cmds = parse(buf, total - 1);
        ssize_t n = cmd_cnt(buf, total - 1);
        run(cmds, n, sockfd);
        wait(NULL);
        close(sockfd);
        exit(0);
    } 
    close(sockfd);
}

