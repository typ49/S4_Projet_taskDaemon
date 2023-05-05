#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

#include "message.h"

#define LINK_PID "/tmp/tasks.txt"
#define LINK_FIFO "/tmp/tasks.fifo"

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

pid_t get_taskd_pid() {
    // testing /tmp/taskd.pid existance with stats
    struct stat statbuf;
    if (stat(LINK_PID, &statbuf) == -1) {
        perror("stat");
        return -1;
    }
    // printing the pid present in the file
    int fc = open(LINK_PID, O_RDONLY);
    if (fc == -1) {
        perror("open");
        return -1;
    }
    char buf[7];// a pid cant be more than 7 characters
                // the maximum pid beeing 2^22 = 4 194 304 (on unix systems)
    int sz = read(fc, buf, 7);
    if ( sz == -1) {
        perror("read");
        return -1;
    }
    char *res = calloc(sz + 1, sizeof(char));
    sprintf(res, "%s", buf);

    pid_t pid = atoi(res);
    free(res);
    return pid;
}

void send_command(int fifo, int argc, char *argv[]) {
    //using send_argv from message.c
    if (send_argv(fifo, argv) == -1) {
        fprintf(stderr, "Error : send_argv\n");
        unlink(LINK_FIFO);
        exit(1);
    }
}

void sender(int argc, char *argv[], time_t st, time_t pe){
    if (mkfifo(LINK_FIFO, 0666)) {
        perror("mkfifo");
        exit(EXIT_FAILURE);
    }

    // -----Waking up the daemon-------
    pid_t pid = get_taskd_pid();
    if(pid == -1) {
        unlink(LINK_FIFO);
        exit(1); // the error is print from the fonction
    }
    kill(pid, SIGUSR1); 
    // --------------------------------    


    //converting the time_t to strings :
    //time_t beeing 2^32 maximum, 10 characters should be enough
    char *start = calloc(11, sizeof(char));
    char *periode = calloc(11, sizeof(char));
    
    sprintf(start, "%ld", st);
    sprintf(periode, "%ld", pe);


    int fifo = open(LINK_FIFO, O_WRONLY);

    
    if (fifo == -1) {
        perror("open");
        unlink(LINK_FIFO);
        exit(EXIT_FAILURE);
    }

    

    send_string(fifo, start);
    send_string(fifo, periode);

    send_command(fifo, argc, argv + 3);
    

    free(start);
    free(periode);

    close(fifo);
    unlink(LINK_FIFO);
}

int main(int argc, char *argv[]) {
    if(argc == 1) {
        // TODO : reading /tmp/tasks.txt
        printf("not done yet\n");
        exit(0);
    }else if(argc < 4) {
        fprintf(stderr, "Error : Invalid number of arguments.\n");
        fprintf(stderr, "Usage : ./taskcli START PERIOD CMD [ARG]...\nUsage : ./taskcli\n");
        exit(1);
    }

    time_t period = get_periode(argv);
    time_t start = get_start(argv);

    sender(argc, argv, start, period);

    return 0;
}