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
    if (regArray.capacity > 0) {
        destroy_registerArray(&regArray);
    }
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

    char **command = recv_argv(fifo); // is free when its container register is destroyed
    if(command == NULL){
        fprintf(stderr, "Error : recv_argv\n");
        exit_program(true);
    }

    //creating the data structure
    struct reg reg = create_register(getAvailableID(), start_time, period_time, command);
    add_register(&regArray, reg);
    
    //open tasks.txt in append mode
    int tasks = open(LINK_TASKS, O_WRONLY | O_APPEND, 0666);
    if(tasks == -1){
        fprintf(stderr, "task open\n");
        exit_program(true);
    }
    char *line = register_to_string(reg);
    if(write(tasks, line, strlen(line)) == -1){
        fprintf(stderr, "task write\n");
        exit_program(true);
    }
    free(line);
    if(close(tasks) == -1){
        fprintf(stderr, "task close\n");
        exit_program(true);
    }

    close(fifo);
}

/**
 * Get the waiting time before the next task
 * 
 * @return the waiting time in seconds -1 if there is no task
*/
time_t get_waitingTime() {
    time_t now = time(NULL);
    time_t min = -1;
    for(size_t i = 0; i < regArray.size; ++i) {
        if (regArray.array[i].start <= now) {
            if (regArray.array[i].period == 0) {
                // this part is very important because without it, the function would
                // return -1 for a period of 0 seconds
                return 0;
            }
            time_t delta = regArray.array[i].period - (now - regArray.array[i].start) % regArray.array[i].period - 1;
            if (delta < min || min == -1) {
                min = delta;
            }
        }
    }
    return min;
}

struct registerArray get_currentTasks(){
    time_t now = time(NULL);
    struct registerArray currentTasks = create_registerArray(regArray.size);
    for(size_t i = 0; i < regArray.size; ++i) {
        if (regArray.array[i].period == 0){
            add_register(&currentTasks, copy_reg(&regArray.array[i]));
        }else{
            time_t delta = regArray.array[i].period - (now - regArray.array[i].start) % regArray.array[i].period - 1;
            if (delta == 0) {
                add_register(&currentTasks, copy_reg(&regArray.array[i]));
            }
        }
    }
    return currentTasks;
}

void sigalrm_handler() {
    struct registerArray currentTasks = get_currentTasks();
    if (currentTasks.size == 0) {
        perror("sigalrm_handler currentTasks.size is null");
        return;
    }
    for (size_t i = 0; i < currentTasks.size; ++i) {
        pid_t pid = fork();
        if (pid == -1) {
            perror("sigalrm_handler fork");
            exit_program(true);
        }
        if (pid == 0) {
            execvp(currentTasks.array[i].cmd[0], currentTasks.array[i].cmd);
            perror("sigalrm_handler execvp");
            exit_program(true);
        }
        if(currentTasks.array[i].period == 0){
            suppress_register(&regArray, currentTasks.array[i].num_cmd);
        }
    }
    destroy_registerArray(&currentTasks);
    while (wait(NULL) != -1);   
    sleep(1);
}




/**
 * init the registerArray with the tasks in tasks.txt
*/
// void init_regArray(){
//     int tasks = open(LINK_TASKS, O_RDONLY);
//     if(tasks == -1){
//         fprintf(stderr, "init_regArray open\n");
//         exit_program(true);
//     }
//     // reading the file's lines one by one
//     // eache line is a task in the format : num_cmd;start;period;cmd
//     char line[100]; // a line can't be longer than 100 characters
//     while(read_line(tasks, line, sizeof(line)) != 0){
//         // parsing the line
//         size_t num_cmd;
//         time_t start;
//         size_t period;
//         char **cmd;
//         if(sscanf(line, "%d;%ld;%zu;", &num_cmd, &start, &period) != 3){
//             fprintf(stderr, "init_regArray sscanf\n");
//             exit_program(true);
//         }
//         cmd = recv_argv(tasks);
//         if(cmd == NULL){
//             fprintf(stderr, "init_regArray recv_argv\n");
//             exit_program(true);
//         }
//         // creating the data structure
//         struct reg reg = create_register(num_cmd, start, period, cmd);
//         add_register(&regArray, reg);
//     }
// }

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

    struct sigaction a3;
    a3.sa_handler = sigalrm_handler;
    a3.sa_flags = 0;
    sigemptyset(&a3.sa_mask);
    sigaction(SIGALRM, &a3, NULL);



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

    time_t waitingTime;
    while(1){
        waitingTime = get_waitingTime();
        if (waitingTime == -1) {
            // no command to execute
            // we wait for a signal to add one
            pause();
        } else if (waitingTime == 0) {
            kill(getpid(), SIGALRM);
        } else {
            alarm(waitingTime);
            // we wait for a signal (SIGALRM)
            pause();
        }
    }
    exit_program(true);
}