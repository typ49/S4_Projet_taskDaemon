#include <stdio.h>
#include <time.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <number of seconds since Epoch>\n", argv[0]);
        return 1;
    }

    time_t t;
    char *str_date;

    t = atol(argv[1]);
    str_date = ctime(&t);

    printf("Date : %s\n", str_date);
    return 0;
}
