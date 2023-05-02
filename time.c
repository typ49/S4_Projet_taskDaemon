#include <stdio.h>
#include <time.h>

int main()
{
    time_t temps;
    temps = time(NULL);
    // printf("Nombre de secondes depuis Epoch : %ld\n", temps);
    printf("%ld\n", temps);
    return 0;
}
