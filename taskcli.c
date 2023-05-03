#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if(argc == 1) {
        // TODO : reading /tmp/tasks.txt
    }else if(argc < 4) {
        fprintf(stderr, "Error : Invalid number of arguments.\n");
        fprintf(stderr, "Usage : ./taskcli START PERIOD CMD [ARG]...\nUsage : ./taskcli");
        exit(1);
    }
    // getting the periode
    char *endptr;
    size_t period = strtol(argv[2], &endptr, 10);
    if(*endptr != '\0') {
        fprintf(stderr, "Invalid preriode : %s", argv[2]);
        fprintf(stderr, "Usage : ./taskcli START PERIOD CMD [ARG]...\nUsage : ./taskcli");
        exit(1);
    }
}