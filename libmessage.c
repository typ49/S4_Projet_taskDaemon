#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

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
    if (write(fd, str, len * sizeof(char)) != len * sizeof(char)) {
        perror("send_string write str");
        return 1;
    }
    return 0;
}

/**
 * Receive a string from a file descriptor.
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
    char *str = malloc(len * sizeof(char) + 1);
    if (str == NULL) {
        perror("recv_string malloc");
        return NULL;
    }
    str[len] = '\0';
    if (read(fd, str, len * sizeof(char)) != len * sizeof(char)) {
        perror("recv_string read str");
        return NULL;
    }
    return str;
}


int main() {
    return 0;
}