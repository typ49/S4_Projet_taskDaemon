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

volatile int usr1_receive = 0;
volatile int alrm_receive = 0;

// Global variables
struct registerArray regArray;


void exit_program(bool error){
    while(wait(NULL) != -1);
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

void kill_handler(){
    exit_program(true);
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

void sigusr1_handler(){
    usr1_receive = 1;
}

void sigalrm_handler(){
    alrm_receive = 1;
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

    //-------------Locking the file------------------
    struct flock lock;
    lock.l_type = F_WRLCK;        // Write lock
    lock.l_whence = SEEK_SET;     // Offset is relative to the start of the file
    lock.l_start = 0;             // Start offset for the lock
    lock.l_len = 0;               // Lock the entire file; 0 means to EOF

    if (fcntl(tasks, F_SETLKW, &lock) == -1) {
        perror("fcntl");
        close(tasks);
        exit(EXIT_FAILURE);
    }
    //-----------------------------------------------

    char *line = register_to_string(reg);
    if(write(tasks, line, strlen(line)) == -1){
        fprintf(stderr, "task write\n");
        exit_program(true);
    }
    free(line);

     //---------Unlocking the file--------------------
    lock.l_type = F_UNLCK;
    if (fcntl(tasks, F_SETLKW, &lock) == -1) {
        perror("fcntl");
        close(tasks);
        exit(EXIT_FAILURE);
    }
    //-----------------------------------------------

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

void sigchld_handler() {
    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        if (WIFEXITED(status)) {
            printf("Le processus fils avec PID %d s'est terminé normalement avec le code de sortie : %d\n", pid, WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("Le processus fils avec PID %d s'est terminé à cause du signal : %d\n", pid, WTERMSIG(status));
        }
    }
}

void execute_tasks() {
    struct registerArray currentTasks = get_currentTasks();
    if (currentTasks.size == 0) {
        perror("execute_tasks currentTasks.size is null");
        return;
    }
    for (size_t i = 0; i < currentTasks.size; ++i) {
        pid_t pid = fork();
        if (pid == -1) {
            perror("execute_tasks fork");
            exit_program(true);
        }
        if (pid == 0) {
            // redirecting stdin to /dev/null
            int devnull = open("/dev/null", O_RDONLY);
            if (devnull == -1) {
                perror("execute_tasks open");
                exit_program(true);
            }
            if (dup2(devnull, fileno(stdin)) == -1) {
                perror("execute_tasks dup2");
                exit_program(true);
            }
            // redirecting stdout to /tmp/tasks/num_cmd.out
            char *path = calloc(sizeof(char), 36); // 20 pour num_cmd + 16 pour [/tmp/tasks/] + [.out] + [\0]
            if (path == NULL) {
                perror("execute_tasks calloc");
                exit_program(true);
            }
            sprintf(path, "/tmp/tasks/%ld.out", currentTasks.array[i].num_cmd);
            int out = open(path, O_WRONLY | O_CREAT | O_APPEND, 0666);
            if (out == -1) {
                perror("execute_tasks open");
                exit_program(true);
            }
            if (dup2(out, fileno(stdout)) == -1) {
                perror("execute_tasks dup2");
                exit_program(true);
            }

            // redirecting stderr to /tmp/tasks/num_cmd.err
            sprintf(path, "/tmp/tasks/%ld.err", currentTasks.array[i].num_cmd);
            int err = open(path, O_WRONLY | O_CREAT | O_APPEND, 0666);
            if (err == -1) {
                perror("execute_tasks open");
                exit_program(true);
            }
            if (dup2(err, fileno(stderr)) == -1) {
                perror("execute_tasks dup2");
                exit_program(true);
            }

            free(path);

            execvp(currentTasks.array[i].cmd[0], currentTasks.array[i].cmd);
            perror("execute_tasks execvp");
            close(devnull);
            exit_program(true);
        }
        if(currentTasks.array[i].period == 0){
            suppress_register(&regArray, currentTasks.array[i].num_cmd);
        }
    }
    destroy_registerArray(&currentTasks);
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
    a2.sa_handler = sigusr1_handler;
    a2.sa_flags = 0;
    sigemptyset(&a2.sa_mask);
    sigaction(SIGUSR1, &a2, NULL);

    struct sigaction a3;
    a3.sa_handler = sigalrm_handler;
    a3.sa_flags = 0;
    sigemptyset(&a3.sa_mask);
    sigaction(SIGALRM, &a3, NULL);

    struct sigaction a4;
    a4.sa_handler = kill_handler;
    a4.sa_flags = 0;
    sigemptyset(&a4.sa_mask);
    sigaction(SIGQUIT, &a4, NULL);

    struct sigaction a5;
    a5.sa_handler = kill_handler;
    a5.sa_flags = 0;
    sigemptyset(&a5.sa_mask);
    sigaction(SIGTERM, &a5, NULL);

    struct sigaction a6;
    a6.sa_handler = sigchld_handler;
    a6.sa_flags = 0;
    sigemptyset(&a6.sa_mask);
    sigaction(SIGCHLD, &a6, NULL);


    // creating the fifo if does not exist
    struct stat statbuf2;
    if (stat(LINK_FIFO, &statbuf2) == -1) {
        if (mkfifo(LINK_FIFO, 0666)) {
            perror("mkfifo");
            exit_program(true);
        }
    }

    //if does not exist, creat /tmp/tasks directory.
    struct stat statbuf3;
    if (stat("/tmp/tasks", &statbuf3) == -1) {
        if (mkdir("/tmp/tasks", 0777)) {
            perror("mkdir");
            exit_program(true);
        }
    }

    //if does not exist, creat /tmp/tasks.txt file.
    struct stat statbuf4;
    if (stat(LINK_TASKS, &statbuf4) == -1) {
        if (creat(LINK_TASKS, 0666)) {
            perror("creat");
            exit_program(true);
        }
    }

    time_t waitingTime;
    time_t last_checked = time(NULL) - 1;
    sigset_t mask;
    sigfillset(&mask);  
    sigdelset(&mask, SIGUSR1);
    sigdelset(&mask, SIGALRM); 
    while(1){
        // checking if there are some signals to handle
        if (usr1_receive) {
            usr1_receive = 0;
            task();            
        }
        if (alrm_receive) {
            alrm_receive = 0;
            execute_tasks();
            // if we just executed the currentTasks, we don't want
            // to check for commands to execute in this timestamp.
            last_checked = time(NULL);
        }
        if (last_checked != time(NULL)) {
            last_checked = time(NULL);
            waitingTime = get_waitingTime();
            printf("waitingTime : %ld\n", waitingTime);
            if (waitingTime == -1) {
                // no command to execute
                // we wait for a signal to add one
                sigsuspend(&mask);
            } else if (waitingTime == 0) {
                kill(getpid(), SIGALRM);
            } else {
                alarm(waitingTime);
                // we wait for a signal (SIGALRM or SIGUSR1)
                sigsuspend(&mask);
            }
        }
    }
    exit_program(true);
}