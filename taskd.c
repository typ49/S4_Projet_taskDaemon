#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>


#include "message.h"

#define PID_PATH "/tmp/tasks.txt"

void sigint_handler(){
    unlink(PID_PATH);
    exit(0);
}

void sigusr1_handler(){
    printf("Coucou !\n");
}

int main() {
    // testing /tmp/taskd.pid existance with stats
    struct stat statbuf;
    if (stat(PID_PATH, &statbuf) != -1) {
        fprintf(stderr, "A process is currently running taskd.\n");
        exit(1);
    }
    
    FILE *f = fopen(PID_PATH, "w");
    pid_t pid = getpid();
    fprintf(f, "%d", pid);
    fclose(f);

    struct sigaction a;
    a.sa_handler = sigint_handler;
    a.sa_flags = 0;
    sigemptyset(&a.sa_mask);
    sigaction(SIGINT, &a, NULL);

    struct sigaction a2;
    a2.sa_handler = sigusr1_handler;
    a2.sa_flags = 0;
    sigemptyset(&a2.sa_mask);
    sigaction(SIGUSR1, &a2, NULL);

    while(1){
        pause();
    }

    unlink(PID_PATH);
    return 1;
}