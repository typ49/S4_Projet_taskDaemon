// compilation: gcc -Wall -g -Wextra -std=c99 -o libmessage libmessage.c
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#include "message.h"


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
    for (size_t i = 0; argv_out[i] != NULL; i++) {
        free(argv_out[i]);
        argv_out[i] = NULL;
    }
    free(argv_out);

    close(fd);
    unlink("baz");
    return 0;
}

