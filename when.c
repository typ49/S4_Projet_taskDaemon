#include <stdio.h>
#include <time.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("Usage: %s <nombre de secondes depuis Epoch>\n", argv[0]);
        return 1;
    }

    time_t temps;
    char *date_str;

    temps = atol(argv[1]);
    date_str = ctime(&temps);

    printf("Date : %s\n", date_str);
    return 0;
}
