#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "message.h"


time_t get_start(char *argv[]){
    char *endptr;
    time_t start;
    if (argv[1][0] == '+') {
        time_t current = time(NULL);
        start = current + strtol(argv[1], &endptr, 10);
        if (*endptr != '\0') {
            fprintf(stderr, "Invalid start : %s\n", argv[1]);
            fprintf(stderr, "Usage : ./taskcli START PERIOD CMD [ARG]...\nUsage : ./taskcli\n");
            exit(1);
        }
    } else {
        start = strtol(argv[1], &endptr, 10);
        if (*endptr != '\0') {
            fprintf(stderr, "Invalid start : %s\n", argv[1]);
            fprintf(stderr, "Usage : ./taskcli START PERIOD CMD [ARG]...\nUsage : ./taskcli\n");
            exit(1);
        }
    }
    return start;
}

time_t get_periode(char *argv[]){
    char *endptr;
    size_t period = strtol(argv[2], &endptr, 10);
    if(*endptr != '\0') {
        fprintf(stderr, "Invalid preriode : %s", argv[2]);
        fprintf(stderr, "Usage : ./taskcli START PERIOD CMD [ARG]...\nUsage : ./taskcli\n");
        exit(1);
    }
    return period;
}

// time_t get_taskd_pid() {
//     // testing /tmp/taskd.pid existance with stats
//     struct stat statbuf;
//     if (stat("/tmp/taskd.pid", &statbuf) == -1) {
//         perror("stat");
//         return 1;
//     }
//     // printing the pid present in the file
//     int pid = open("/tmp/taskd.pid", O_RDONLY);
//     if (pid == -1) {
//         perror("open");
//         return 1;
//     }
//     char buf[10];
//     int sz = read(pid, buf, 10);
//     if ( sz == -1) {
//         perror("read");
//         return 1;
//     }
//     char *res = calloc(sz + 1, sizeof(char));
//     sprintf(res, "%s", buf);
//     return res;
// }

int main(int argc, char *argv[]) {
    if(argc == 1) {
        // TODO : reading /tmp/tasks.txt
        exit(0);
    }else if(argc < 4) {
        fprintf(stderr, "Error : Invalid number of arguments.\n");
        fprintf(stderr, "Usage : ./taskcli START PERIOD CMD [ARG]...\nUsage : ./taskcli\n");
        exit(1);
    }

    time_t period = get_periode(argv);
    time_t start = get_start(argv);

    // on reveille taskd :
    
    return 0;
}