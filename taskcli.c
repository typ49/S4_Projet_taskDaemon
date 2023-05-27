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

#define LINK_PID "/tmp/tasks.pid"
#define LINK_FIFO "/tmp/tasks.fifo"
#define LINK_TASKS "/tmp/tasks.txt"

time_t get_start(char *argv[]){
    char *endptr;
    time_t start;
    if (argv[1][0] == '+') {
        time_t current = time(NULL);
        start = current + strtol(argv[1], &endptr, 10);
        if (*endptr != '\0') {
            fprintf(stderr, "Invalid start : %s\n", argv[1]);
            fprintf(stderr, "Usage : ./taskcli START PERIOD CMD [ARG]...\nUsage : ./taskcli -d numCommandLine\nUsage : ./taskcli\n");
            exit(1);
        }
    } else {
        start = strtol(argv[1], &endptr, 10);
        if (*endptr != '\0') {
            fprintf(stderr, "Invalid start : %s\n", argv[1]);
            fprintf(stderr, "Usage : ./taskcli START PERIOD CMD [ARG]...\nUsage : ./taskcli -d numCommandLine\nUsage : ./taskcli\n");
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
        fprintf(stderr, "Usage : ./taskcli START PERIOD CMD [ARG]...\nUsage : ./taskcli -d numCommandLine\nUsage : ./taskcli\n");
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

void send_command(int fifo, char *argv[]) {
    //using send_argv from message.c
    if (send_argv(fifo, argv) == -1) {
        fprintf(stderr, "Error : send_argv\n");
        exit(1);
    }
}

void sender(char *argv[], time_t st, time_t pe){
    

    // -----Waking up the daemon-------
    pid_t pid = get_taskd_pid();
    if(pid == -1) {
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
        exit(EXIT_FAILURE);
    }

    //-------------Locking the fifo------------------
    struct flock lock;
    lock.l_type = F_WRLCK;        // Write lock
    lock.l_whence = SEEK_SET;     // Offset is relative to the start of the file
    lock.l_start = 0;             // Start offset for the lock
    lock.l_len = 0;               // Lock the entire file; 0 means to EOF

    if (fcntl(fifo, F_SETLKW, &lock) == -1) {
        perror("fcntl");
        close(fifo);
        exit(EXIT_FAILURE);
    }
    //-----------------------------------------------

    send_string(fifo, start);
    send_string(fifo, periode);

    send_command(fifo, argv + 3);
    
    //---------Unlocking the fifo--------------------
    lock.l_type = F_UNLCK;
    if (fcntl(fifo, F_SETLKW, &lock) == -1) {
        perror("fcntl");
        close(fifo);
        exit(EXIT_FAILURE);
    }
    //-----------------------------------------------

    free(start);
    free(periode);

    close(fifo);
}

void read_tasks() {
    int fd = open(LINK_TASKS, O_RDONLY);
    if (fd == -1) {
        perror("open tasks.txt");
        exit(EXIT_FAILURE);
    }

    //-------------Locking the file------------------
    struct flock lock;
    lock.l_type = F_RDLCK;        // Read lock
    lock.l_whence = SEEK_SET;     // Offset is relative to the start of the file
    lock.l_start = 0;             // Start offset for the lock
    lock.l_len = 0;               // Lock the entire file; 0 means to EOF

    if (fcntl(fd, F_SETLKW, &lock) == -1) {
        perror("fcntl");
        close(fd);
        exit(EXIT_FAILURE);
    }
    //-----------------------------------------------

    char buf[1024];
    ssize_t sz;
    while((sz = read(fd, buf, sizeof(buf) - 1)) > 0) {
        buf[sz] = '\0'; // ensure null-termination
        printf("%s", buf);
    }

    if (sz == -1) {
        perror("read");
        exit(EXIT_FAILURE);
    }

    //---------Unlocking the file--------------------
    lock.l_type = F_UNLCK;
    if (fcntl(fd, F_SETLKW, &lock) == -1) {
        perror("fcntl");
        close(fd);
        exit(EXIT_FAILURE);
    }
    //-----------------------------------------------

    if(close(fd) == -1) {
        perror("close");
        exit(EXIT_FAILURE);
    }
}


int main(int argc, char *argv[]) {
    if(argc == 1) {
        read_tasks();
        exit(0);
    }else if (argc == 3) {
        if(strcmp(argv[1], "-d") == 0) {
            //on vérifie que argv[2] est bien un nombre
            if (argv[2][0] < '0' || argv[2][0] > '9') {
                fprintf(stderr, "Error : Invalid number of command line.\n");
                fprintf(stderr, "Usage : ./taskcli START PERIOD CMD [ARG]...\nUsage : ./taskcli -d numCommandLine\n : ./taskcli\n");
                exit(1);
            }
            // on récupère le pid de taskd
            pid_t pid = get_taskd_pid();
            if(pid == -1) {
                exit(1); // the error is print from the fonction
            }
            // on envoie le signal SIGUSR2 au daemon
            kill(pid, SIGUSR2);
            // on envoie l'entier num dans le fifo
            int fifo = open(LINK_FIFO, O_WRONLY);

    
            if (fifo == -1) {
                perror("open");
                exit(EXIT_FAILURE);
            }

            //-------------Locking the fifo------------------
            struct flock lock;
            lock.l_type = F_WRLCK;        // Write lock
            lock.l_whence = SEEK_SET;     // Offset is relative to the start of the file
            lock.l_start = 0;             // Start offset for the lock
            lock.l_len = 0;               // Lock the entire file; 0 means to EOF

            if (fcntl(fifo, F_SETLKW, &lock) == -1) {
                perror("fcntl");
                close(fifo);
                exit(EXIT_FAILURE);
            }
            //-----------------------------------------------
            send_string(fifo, argv[2]);
            //---------Unlocking the fifo--------------------
            lock.l_type = F_UNLCK;
            if (fcntl(fifo, F_SETLKW, &lock) == -1) {
                perror("fcntl");
                close(fifo);
                exit(EXIT_FAILURE);
            }
            //-----------------------------------------------
            close(fifo);

        }

            

        
    }else if(argc < 4) {
        fprintf(stderr, "Error : Invalid number of arguments.\n");
        fprintf(stderr, "Usage : ./taskcli START PERIOD CMD [ARG]...\nUsage : ./taskcli -d numCommandLine\n : ./taskcli\n");
        exit(1);
    }

    time_t period = get_periode(argv);
    time_t start = get_start(argv);

    sender(argv, start, period);

    return 0;
}