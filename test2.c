// compilation: gcc -Wall -g -Wextra -std=c99 -o libmessage libmessage.c
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

/**
 * Send a string to a file descriptor.
 * 
 * @param fd The file descriptor to write to.
 * @param str The string to write.
 * 
 * @return 0 on success, 1 on failure.
*/
int send_string(int fd, char *str) {
    size_t len = strlen(str);
    if (write(fd, &len, sizeof(size_t)) != sizeof(size_t)) {
        perror("send_string write len");
        return 1;
    }
    if (write(fd, str, len) != (ssize_t)len) {
        perror("send_string write str");
        return 1;
    }
    return 0;
}

/**
 * Receive a string from a file descriptor.
 * Be careful to free the string when you're done with it.
 * 
 * @param fd The file descriptor to read from.
 * 
 * @return The string read from the file descriptor.
 */
char *recv_string(int fd) {
    size_t len;
    if (read(fd, &len, sizeof(size_t)) != sizeof(size_t)) {
        perror("recv_string read len");
        return NULL;
    }
    char *str = malloc((len + 1) * sizeof(char));
    if (str == NULL) {
        perror("recv_string malloc");
        return NULL;
    }
    str[len] = '\0';
    if (read(fd, str, len) != (ssize_t)len) {
        perror("recv_string read str");
        return NULL;
    }
    return str;
}

/**
 * Send an array of strings to a file descriptor.
 * 
 * @param fd The file descriptor to write to.
 * @param argv The array of strings to write.
 * 
 * @return 0 on success, 1 on failure.
 */
int send_argv(int fd, char *argv[]) {
    size_t len = 0;
    while (argv[len] != NULL) {
        len++;
    }
    if (write(fd, &len, sizeof(size_t)) != sizeof(size_t)) {
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


/**
 * Receive an array of strings from a file descriptor.
 * Be careful to free the array and its strings when you're done with it.
 * 
 * @param fd The file descriptor to read from.
 * 
 * @return The array of strings read from the file descriptor.
 */
char **recv_argv(int fd) {
    size_t len;
    if (read(fd, &len, sizeof(size_t)) != sizeof(size_t)) {
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


int main() {
    int fd = open("baz", O_RDONLY);
    if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    char **argv_out = recv_argv(fd);
    for (size_t i = 0; argv_out[i] != NULL; i++) {
        printf("%s\n", argv_out[i]);
    }
    free(argv_out);

    close(fd);
    unlink("baz");
    return 0;
}

