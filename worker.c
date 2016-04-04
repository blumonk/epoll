#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

#include "worker.h"

int exec(struct command *cmd) {
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

int run(struct command** commands, size_t n) {
    int pipes[2 * n];
    int pids[n];
    int this_stdin = dup(STDIN_FILENO);
    int this_stdout = dup(STDOUT_FILENO);
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
    SAFE(dup2(this_stdout, STDOUT_FILENO));
    SAFE(close(this_stdout));
    pids[n - 1] = exec(commands[n - 1]);
    SAFE(pids[n - 1]);
    wait(NULL);
    return 0;
}

