#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
// for read
#include <unistd.h>


int main() {
    // testing /tmp/taskd.pid existance with stats
    struct stat statbuf;
    if (stat("/tmp/taskd.pid", &statbuf) == -1) {
        perror("stat");
        return 1;
    }
    // printing the pid present in the file
    int pid = open("/tmp/taskd.pid", O_RDONLY);
    if (pid == -1) {
        perror("open");
        return 1;
    }
    char buf[10];
    if (read(pid, buf, 10) == -1) {
        perror("read");
        return 1;
    }
    printf("%s\n", buf);
}