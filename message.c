// compilation: gcc -Wall -g -Wextra -std=c99 -o message message.c
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "message.h"

int send_string(int fd, char *str) {
    size_t len = strlen(str);
    if (write(fd, &len, sizeof(size_t)) == -1) {
        perror("send_string write len");
        return 1;
    }
    if (write(fd, str, len) == -1) {
        perror("send_string write str");
        return 1;
    }
    return 0;
}


char *recv_string(int fd) {
    size_t len;
    if (read(fd, &len, sizeof(size_t)) == -1) {
        perror("recv_string read len");
        return NULL;
    }
    char *str = malloc((len + 1) * sizeof(char));
    if (str == NULL) {
        perror("recv_string malloc");
        return NULL;
    }
    str[len] = '\0';
    if (read(fd, str, len) == -1) {
        perror("recv_string read str");
        return NULL;
    }
    return str;
}


int send_argv(int fd, char *argv[]) {
    size_t len = 0;
    while (argv[len] != NULL) {
        len++;
    }
    if (write(fd, &len, sizeof(size_t)) == -1) {
        perror("send_argv write len");
        return 1;
    }
    for (size_t i = 0; i < len; i++) {
        if (send_string(fd, argv[i]) != 0) {
            return 1;
        }
    }
    return 0;
}


char **recv_argv(int fd) {
    size_t len;
    if (read(fd, &len, sizeof(size_t)) == -1) {
        perror("recv_argv read len");
        return NULL;
    }
    char **argv = malloc((len + 1) * sizeof(char *));
    if (argv == NULL) {
        perror("recv_argv malloc");
        return NULL;
    }
    for (size_t i = 0; i < len; i++) {
        argv[i] = recv_string(fd);
        if (argv[i] == NULL) {
            for (size_t j = 0; j < i; j++) {
                free(argv[j]);
            }
            free(argv);
            return NULL;
        }
    }
    argv[len] = NULL;
    return argv;
}