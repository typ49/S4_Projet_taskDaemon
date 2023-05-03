// compilation: gcc -Wall -g -Wextra -std=c99 -o libmessage libmessage.c
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#include "message.h"




int main() {
   
    if (mkfifo("baz", 0644)) {
        perror("mkfifo");
        exit(EXIT_FAILURE);
    }

    int fd = open("baz", O_WRONLY);
    if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    char **argv_in = malloc(10 * sizeof(char *)); // todo : free
    argv_in[0] = "Hello";
    argv_in[1] = "World";
    argv_in[2] = "!";
    argv_in[3] = "This";
    argv_in[4] = "is";
    argv_in[5] = "a";
    argv_in[6] = "test";
    argv_in[7] = "of";
    argv_in[8] = "the";
    argv_in[9] = NULL;

    send_argv(fd, argv_in);

    free(argv_in);
    close(fd);
    unlink("baz");
    return 0;
}

