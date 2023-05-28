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
#include <dirent.h>


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
        fprintf(stderr, "Error : recv_argv");
        exit_program(true);
    }

    //creating the data structure
    struct reg reg;
    create_register(&reg, getAvailableID(), start_time, period_time, command);
    add_register(&regArray, reg);

    //open tasks.txt in append mode
    int tasks = open(LINK_TASKS, O_WRONLY | O_APPEND , 0666);
    if(tasks == -1){
        fprintf(stderr, "task open\n");
        exit_program(true);
    }

    //-------------Locking the file------------------
    struct flock lock2;
    lock2.l_type = F_WRLCK;        // Write lock
    lock2.l_whence = SEEK_SET;     // Offset is relative to the start of the file
    lock2.l_start = 0;             // Start offset for the lock
    lock2.l_len = 0;               // Lock the entire file; 0 means to EOF

    if (fcntl(tasks, F_SETLKW, &lock2) == -1) {
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
    lock2.l_type = F_UNLCK;
    if (fcntl(tasks, F_SETLKW, &lock2) == -1) {
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
 * @brief Removes all the content of a directory
 * 
 * @param path Path to the directory
*/

int remove_directory_content(const char *path) {
    DIR *d = opendir(path);
    size_t path_len = strlen(path);
    int r = -1;

    if (d) {
        struct dirent *p;
        r = 0;

        while (r == 0 && (p = readdir(d))) {
            int r2 = -1;
            char *buf;
            size_t len;

            /* Skip the names "." and ".." as we don't want to recurse on them. */
            if (strcmp(p->d_name, ".") == 0 || strcmp(p->d_name, "..") == 0)
                continue;

            len = path_len + strlen(p->d_name) + 2; 
            buf = malloc(len);

            if (buf) {
                struct stat statbuf;
                snprintf(buf, len, "%s/%s", path, p->d_name);

                if (!stat(buf, &statbuf)) {
                    if (!S_ISDIR(statbuf.st_mode)) // check if it is not a directory
                        r2 = unlink(buf);
                }

                free(buf);
            }

            r = r2;
        }

        closedir(d);
    }

    return r;
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
        } else {
            // regArray.array[i].start > now
            time_t delta = regArray.array[i].start - now - 1;
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

    pid = wait(NULL);
    if (pid > 0) {
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
            destroy_registerArray(&currentTasks);
            exit_program(true);
        }
        if (pid == 0) {
            // redirecting stdin to /dev/null
            int devnull = open("/dev/null", O_RDONLY);
            if (devnull == -1) {
                perror("execute_tasks open");
                destroy_registerArray(&currentTasks);
                exit_program(true);
            }
            if (dup2(devnull, fileno(stdin)) == -1) {
                perror("execute_tasks dup2");
                destroy_registerArray(&currentTasks);
                exit_program(true);
            }
            // redirecting stdout to /tmp/tasks/num_cmd.out
            char *path = calloc(sizeof(char), 36); // 20 pour num_cmd + 16 pour [/tmp/tasks/] + [.out] + [\0]
            if (path == NULL) {
                perror("execute_tasks calloc");
                destroy_registerArray(&currentTasks);
                exit_program(true);
            }
            sprintf(path, "/tmp/tasks/%ld.out", currentTasks.array[i].num_cmd);
            int out = open(path, O_WRONLY | O_CREAT | O_APPEND, 0666);
            if (out == -1) {
                perror("execute_tasks open");
                destroy_registerArray(&currentTasks);
                exit_program(true);
            }
            if (dup2(out, fileno(stdout)) == -1) {
                perror("execute_tasks dup2");
                destroy_registerArray(&currentTasks);
                exit_program(true);
            }

            // redirecting stderr to /tmp/tasks/num_cmd.err
            sprintf(path, "/tmp/tasks/%ld.err", currentTasks.array[i].num_cmd);
            int err = open(path, O_WRONLY | O_CREAT | O_APPEND, 0666);
            if (err == -1) {
                perror("execute_tasks open");
                destroy_registerArray(&currentTasks);
                exit_program(true);
            }
            if (dup2(err, fileno(stderr)) == -1) {
                perror("execute_tasks dup2");
                destroy_registerArray(&currentTasks);
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


// retire une commande de la liste courante des commandes à exécuter avec l’outil taskcli :
// 1. Supprimer la commande de la liste des commandes à exécuter.
// 2. Supprimer les fichiers de sortie de la commande.
// 3. Supprimer la ligne correspondante dans le fichier tasks.txt.
void delete_command (size_t index) {
    if (index >= regArray.size) {
        fprintf(stderr, "Error : delete_command index out of range\n");
        exit_program(true);
    }
    // 1. Supprimer la commande de la liste des commandes à exécuter.
    suppress_register(&regArray, regArray.array[index].num_cmd);
    // 2. Supprimer les fichiers de sortie de la commande.
    char *path = calloc(sizeof(char), 36); // 20 pour num_cmd + 16 pour [/tmp/tasks/] + [.out] + [\0]
    if (path == NULL) {
        perror("delete_command calloc");
        exit_program(true);
    }
    sprintf(path, "/tmp/tasks/%ld.out", regArray.array[index].num_cmd);
    if (unlink(path) == -1) {
        perror("delete_command unlink");
        exit_program(true);
    }
    sprintf(path, "/tmp/tasks/%ld.err", regArray.array[index].num_cmd);
    if (unlink(path) == -1) {
        perror("delete_command unlink");
        exit_program(true);
    }
    free(path);
    // 3. Supprimer la ligne correspondante dans le fichier tasks.txt.
    int tasks = open(LINK_TASKS, O_RDWR, 0666);
    if (tasks == -1) {
        perror("delete_command open");
        exit_program(true);
    }
    //-------------Locking the file------------------
    struct flock lock;
    lock.l_type = F_WRLCK;        // Write lock
    lock.l_whence = SEEK_SET;     // Offset is relative to the start of the file
    lock.l_start = 0;             // Start offset for the lock
    lock.l_len = 0;               // Lock the entire file; 0 means to EOF

    if (fcntl(tasks, F_SETLKW, &lock) == -1) {
        perror("delete_command fcntl");
        close(tasks);
        exit(EXIT_FAILURE);
    }
    //-----------------------------------------------
    char *line = register_to_string(regArray.array[index]);
    if (line == NULL) {
        perror("delete_command register_to_string");
        exit_program(true);
    }
    if (lseek(tasks, 0, SEEK_SET) == -1) {
        perror("delete_command lseek");
        exit_program(true);
    }
    char buffer[100];
    while (read(tasks, buffer, sizeof(buffer)) > 0) {
        if (strstr(buffer, line) != NULL) {
            if (lseek(tasks, -strlen(buffer), SEEK_CUR) == -1) {
                perror("delete_command lseek");
                exit_program(true);
            }
            if (write(tasks, "\n", 1) == -1) {
                perror("delete_command write");
                exit_program(true);
            }
            break;
        }
    }
    free(line);
    //---------Unlocking the file--------------------
    lock.l_type = F_UNLCK;
    if (fcntl(tasks, F_SETLKW, &lock) == -1) {
        perror("delete_command fcntl");
        close(tasks);
        exit(EXIT_FAILURE);
    }
    //-----------------------------------------------
    if (close(tasks) == -1) {
        perror("delete_command close");
        exit_program(true);
    }
}

int main() {
    // testing /tmp/taskd.pid existance with statsu 0 en cas de succ
    struct stat statbuf;
    if (stat(LINK_PID, &statbuf) != -1) {
        fprintf(stderr, "Error : A process is currently running taskd.\n");
        exit(EXIT_FAILURE);
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

    struct sigaction a7;
    a7.sa_handler = sigchld_handler;
    a7.sa_flags = 0;
    sigemptyset(&a7.sa_mask);
    sigaction(SIGUSR2, &a7, NULL);

    //redirecting stdout to /tmp/taskd.out
    char *path = "/tmp/taskd.out";
    int out = open(path, O_WRONLY | O_CREAT | O_APPEND, 0666);
    if (out == -1) {
        perror("execute_tasks open");
        exit_program(true);
    }
    if (dup2(out, fileno(stdout)) == -1) {
        perror("execute_tasks dup2");
        exit_program(true);
    }

    // redirecting stderr to /tmp/taskd.err
    path = "/tmp/taskd.err";
    int err = open(path, O_WRONLY | O_CREAT | O_APPEND, 0666);
    if (err == -1) {
        perror("execute_tasks open");
        exit_program(true);
    }
    if (dup2(err, fileno(stderr)) == -1) {
        perror("execute_tasks dup2");
        exit_program(true);
    }


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
    //for now, we will empty the file if it exists.
    int tasks = open(LINK_TASKS, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (tasks == -1) {
        perror("open");
        exit_program(true);
    }
    if (close(tasks) == -1) {
        perror("close");
        exit_program(true);
    }
    //also for now, we will empty the /tmp/tasks directory.
    int r = remove_directory_content("/tmp/tasks");
    if(r != 0){
        fprintf(stderr, "Error : remove_directory_content : /tmp/tasks\n");
        exit_program(true);
    }


    time_t waitingTime;
    time_t last_checked = time(NULL) - 1;
    sigset_t mask;
    sigfillset(&mask);  
    sigdelset(&mask, SIGUSR1);
    sigdelset(&mask, SIGALRM); 
    sigdelset(&mask, SIGINT);
    sigdelset(&mask, SIGQUIT);
    sigdelset(&mask, SIGTERM);
    sigdelset(&mask, SIGCHLD);
    sigdelset(&mask, SIGUSR2);
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