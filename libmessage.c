#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>


int send_string(int fd, char *str) {
    size_t len = strlen(str);
    if (write(fd, &len, sizeof(len)) != sizeof(len)) {
        perror("send_string write len");
        exit(1);
    }
    if (write(fd, str, len) != len) {
        perror("send_string write str");
        exit(1);
    }
}


int main() {
    return 0;
}