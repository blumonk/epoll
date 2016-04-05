#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "parse.h"
#include "worker.h"

ssize_t args_cnt(char *buffer, ssize_t from, ssize_t to) {
    ssize_t i, res = 0;
    int inside = 0;
    char quot = 0;
    for (i = from; i < to; ++i) {
        if (buffer[i] != ' ') {
            if (!inside) {
                inside = 1;
                ++res;
                if (buffer[i] == '\'' || buffer[i] == '\"') 
                    quot = buffer[i];
            } else {
                if (buffer[i] == '\'' || buffer[i] == '\"') 
                    quot = inside = 0;
            }
        } else {
            if (quot == 0) 
                inside = 0;
        } 
    }
    return res;
}

// from included, to excluded
command* parse_one(char *buffer, ssize_t from, ssize_t to) {
    ssize_t i;
    command* cmd = (command*) malloc(sizeof(command*));

    while (buffer[from] == ' ' && from < to)
        ++from;
    int parsed_file = 0;
    for (i = from; i < to; ++i) {
        if (buffer[i] == ' ') {
            char *file = (char*) malloc(sizeof(char) * (i - from + 1));
            memcpy(file, buffer + from, i - from);
            file[i - from] = '\0';
            cmd->file = file;
            parsed_file = 1;
            break;
        }
    }
    if (!parsed_file) {
        char *file = (char*) malloc(sizeof(char) * (to - from + 1));
        memcpy(file, buffer + from, to - from);
        file[to - from] = '\0';
        cmd->file = file;
    }

    ssize_t cnt = args_cnt(buffer, from, to);
    char **args = (char**) malloc(sizeof(char*) * (cnt + 1));
    ssize_t j = 0, start = 0;
    int inside = 0;
    char quot = 0;
    for (i = from; i < to; ++i) {
        if (buffer[i] != ' ') {
            if (!inside) {
                inside = 1;
                start = i;
                if (buffer[i] == '\'' || buffer[i] == '\"') 
                    quot = buffer[i];
            } else {
                if (buffer[i] == quot && buffer[i - 1] != '\\') {
                    char* arg = (char*) malloc(sizeof(char) * (i - start));
                    memcpy(arg, buffer + start + 1, i - start - 1);
                    arg[i - start - 1] = '\0';
                    args[j++] = arg;
                    quot = inside = 0;
                }
            }
        } else {
            if (quot == 0) {
                if (inside) {
                    char* arg = (char*) malloc(sizeof(char) * (i - start + 1));
                    memcpy(arg, buffer + start, i - start);
                    arg[i - start] = '\0';
                    args[j++] = arg;
                }
                inside = 0;
            }
        } 
    }
    if (inside) {
        char* arg = (char*) malloc(sizeof(char) * (to - start + 1));
        memcpy(arg, buffer + start, to - start);
        arg[to - start] = '\0';
        args[j] = arg;
    }

    cmd->args = args;
    return cmd;
}

ssize_t cmd_cnt(char *buffer, ssize_t size) {
    ssize_t n = 1, i = 0;
    for (i = 0; i < size; ++i) 
        if (buffer[i] == '|')
            ++n;
    return n;
}

command** parse(char *buffer, ssize_t size) {
    ssize_t cnt = cmd_cnt(buffer, size);
    command** result = (command**) malloc(sizeof(command*) * cnt);
    ssize_t start = 0, j = 0, i; 
    for (i = 0; i < size; ++i) {
        if (buffer[i] == '|') {
            result[j++] = parse_one(buffer, start, i);
            start = i + 1;
        }
    }
    result[j] = parse_one(buffer, start, size);
    return result;
}

