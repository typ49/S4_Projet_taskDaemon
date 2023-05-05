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

#define LINK_PID "/tmp/tasks.txt"
#define LINK_FIFO "/tmp/tasks.fifo"

void sigint_handler(){
    unlink(LINK_PID);
    exit(0);
}

void task(){
    int fifo = open(LINK_FIFO, O_RDONLY);
    if(fifo == -1){
        fprintf(stderr, "Error : open\n");
        unlink(LINK_PID);
        exit(1);
    }

    char *start = recv_string(fifo);
    if(start == NULL){
        fprintf(stderr, "Error : recv_string\n");
        unlink(LINK_PID);
        exit(1);
    }
    char *period = recv_string(fifo);
    if(period == NULL){
        fprintf(stderr, "Error : recv_string\n");
        free(start);
        unlink(LINK_PID);
        exit(1);
    }
    char **commmand = recv_argv(fifo);
    if(commmand == NULL){
        fprintf(stderr, "Error : recv_argv\n");
        free(start);
        free(period);
        unlink(LINK_PID);
        exit(1);
    }

    printf("start : %s\n", start);
    printf("period : %s\n", period);
    printf("command : ");
    for(size_t i = 0; commmand[i] != NULL; ++i) {
        printf("%s ", commmand[i]);
    }
    printf("\n");


    //----------free memory------------------------
    free(start);
    free(period);
    for(size_t i = 0; commmand[i] != NULL; ++i) {
        free(commmand[i]);
    }
    free(commmand);
    //---------------------------------------------

    close(fifo);
}

void sigusr1_handler(){
    task();
}

int main() {
    // testing /tmp/taskd.pid existance with stats
    struct stat statbuf;
    if (stat(LINK_PID, &statbuf) != -1) {
        fprintf(stderr, "Error : A process is currently running taskd.\n");
        exit(1);
    }
    
    FILE *f = fopen(LINK_PID, "w");
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

    unlink(LINK_PID);
    return 1;
}