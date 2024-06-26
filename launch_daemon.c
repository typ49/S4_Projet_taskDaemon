#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

/**
 * Launch the daemon given in argument
*/
int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s ABSOLUTE_PATH_TO_TASKD\n", argv[0]);
        return 1;
    }

    // // setup the library path
    // const char* variableName = "LD_LIBRARY_PATH";
    // const char* variableValue = "/home/bido/licence/SYS/S4_Projet_taskDaemon/";
    //
    // int result = setenv(variableName, variableValue, 1);
    // if (result != 0) {
    //     return 1;
    // }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid > 0) {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
        return 0;
    }

    // Child process
    setsid(); // Create a new session and become the session leader

    pid_t pid2 = fork();
    if (pid2 < 0) {
        perror("fork");
        return 1;
    }

    if (pid2 > 0) {
        // First child process terminates, leaving the grandchild as the daemon
        printf("Daemon process started with PID %d.\n", pid2);
        exit(0);
    }

    // Grandchild process (daemon)
    if (chdir("/") == -1) {
        perror("chdir");
        return 1;
    }

    // Execute the taskd program
    execl(argv[1], argv[1], NULL);
    perror("execl");
    return 1;
}
