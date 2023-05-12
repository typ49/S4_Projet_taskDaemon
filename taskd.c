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
#include <stdbool.h>


#include "message.h"
#include "data.h"

#define LINK_PID "/tmp/tasks.pid"
#define LINK_FIFO "/tmp/tasks.fifo"
#define LINK_TASKS "/tmp/tasks.txt"

// Global variables
struct registerArray regArray;


void exit_program(bool error){
    unlink(LINK_PID);
    destroy_registerArray(&regArray);
    if(error){
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}

void sigint_handler(){
    exit_program(false);
}

int getAvailableID() {
    FILE* file = fopen(LINK_TASKS, "r");
    if (file == NULL) {
        perror("getAvailableID fopen");
        exit_program(true);
    }

    int maxID = 0;
    char line[100];

    while (fgets(line, sizeof(line), file)) {
        int currentID;
        sscanf(line, "%d;", &currentID);

        if (currentID > maxID) {
            maxID = currentID;
        }
    }

    if(fclose(file) == -1){
        perror("getAvailableID fclose");
        exit_program(true);
    }

    return maxID + 1;
}

void task(){
    int fifo = open(LINK_FIFO, O_RDONLY);
    if(fifo == -1){
        fprintf(stderr, "Error : open\n");
        exit_program(true);
    }

    char *start = recv_string(fifo);
    if(start == NULL){
        fprintf(stderr, "Error : recv_string\n");
        exit_program(true);
    }
    time_t start_time = atol(start);
    free(start);

    char *period = recv_string(fifo);
    if(period == NULL){
        fprintf(stderr, "Error : recv_string\n");
        exit_program(true);
    }
    size_t period_time = atol(period);
    free(period);

    char **commmand = recv_argv(fifo);
    if(commmand == NULL){
        fprintf(stderr, "Error : recv_argv\n");
        exit_program(true);
    }

    //creating the data structure
    struct reg reg = create_register(getAvailableID(), start_time, period_time, commmand);
    add_register(&regArray, reg);
    
    //open tasks.txt in append mode
    int tasks = open(LINK_TASKS, O_WRONLY | O_APPEND, 0666);
    if(tasks == -1){
        fprintf(stderr, "task open\n");
        exit_program(true);
    }
    char *line = register_to_string(reg);
    printf("line : %s", line);
    if(write(tasks, line, strlen(line)) == -1){
        fprintf(stderr, "task write\n");
        exit_program(true);
    }
    free(line);
    if(close(tasks) == -1){
        fprintf(stderr, "task close\n");
        exit_program(true);
    }


    //----------free memory------------------------
    for(size_t i = 0; commmand[i] != NULL; ++i) {
        free(commmand[i]);
    }
    free(commmand);
    //---------------------------------------------

    close(fifo);
}

int main() {
    // testing /tmp/taskd.pid existance with stats
    struct stat statbuf;
    if (stat(LINK_PID, &statbuf) != -1) {
        fprintf(stderr, "Error : A process is currently running taskd.\n");
        exit_program(true);
    }
    
    FILE *f = fopen(LINK_PID, "w");
    pid_t pid = getpid();
    fprintf(f, "%d", pid);
    if(fclose(f) == -1){
        fprintf(stderr, "Error : fclose\n");
        exit_program(true);
    }

    // creating the registerArray
    regArray = create_registerArray(10);
    

    struct sigaction a;
    a.sa_handler = sigint_handler;
    a.sa_flags = 0;
    sigemptyset(&a.sa_mask);
    sigaction(SIGINT, &a, NULL);

    struct sigaction a2;
    a2.sa_handler = task;
    a2.sa_flags = 0;
    sigemptyset(&a2.sa_mask);
    sigaction(SIGUSR1, &a2, NULL);


    // creating the fifo if does not exist
    struct stat statbuf2;
    if (stat(LINK_FIFO, &statbuf2) == -1) {
        if (mkfifo(LINK_FIFO, 0666)) {
            perror("mkfifo");
            exit_program(true);
        }
    }

    //S’il n’existe pas, créer le répertoire /tmp/tasks.
    struct stat statbuf3;
    if (stat("/tmp/tasks", &statbuf3) == -1) {
        if (mkdir("/tmp/tasks", 0777)) {
            perror("mkdir");
            exit_program(true);
        }
    }

    //S’il n’existe pas, créer le fichier /tmp/tasks.txt.
    struct stat statbuf4;
    if (stat(LINK_TASKS, &statbuf4) == -1) {
        if (creat(LINK_TASKS, 0666)) {
            perror("creat");
            exit_program(true);
        }
    }


    while(1){
        pause();
    }
    exit_program(true);
}