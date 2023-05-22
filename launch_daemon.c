#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s ABSOLUTE_PATH_TO_TASKD\n", argv[0]);
        return 1;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid > 0) {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
        printf("Daemon process started with PID %d.\n", pid);
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
        exit(0);
    }

    // Grandchild process (daemon)
    umask(0); // Set file mode creation mask to 0
    if (chdir("/") == -1) {
        perror("chdir");
        return 1;
    }

    // Close all open file descriptors
    for (int fd = sysconf(_SC_OPEN_MAX); fd > 0; --fd) {
        close(fd);
    }

    // Execute the taskd program
    execl(argv[1], argv[1], NULL);
    perror("execl");
    return 1;
}
