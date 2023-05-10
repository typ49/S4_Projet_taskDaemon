#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "data.h"


struct reg create_register(size_t num_cmd, time_t start, size_t period, char **cmd)
{
    struct reg reg;
    reg.num_cmd = num_cmd;
    reg.start = start;
    reg.period = period;
    reg.cmd = cmd;
    return reg;
}

char *register_to_string(struct reg reg)
{
    // calculate the size of the string
    // 1. size of the command
    size_t len = 0;
    for (size_t i = 0; reg.cmd[i] != NULL; ++i)
    {
        len += strlen(reg.cmd[i]) + 1;
    }
    // 2. size the num_cmd and period fields
    len += sizeof(size_t);
    len += sizeof(time_t);
    // 3. size of the start fieldh
    len += 21; // a date with the maximum size beeing : '10 septembre 23:59:59'
    // 4. size of the ';' and '\0'
    len += 4;

    // create the string with calloc so no need to add the '\0' at the end
    char *res = calloc(len, sizeof(char));

    // add the num_cmd field
    sprintf(res, "%zu", reg.num_cmd);
    strcat(res, ";");

    // add the start field
    char *start = ctime(&reg.start);
    start[strlen(start) - 1] = '\0'; // remove the '\n' at the end
    strcat(res, start);

    // add the period field
    strcat(res, ";");
    sprintf(res + strlen(res), "%zu", reg.period);

    // add the command field
    strcat(res, ";");
    for (size_t i = 0; reg.cmd[i] != NULL; ++i)
    {
        strcat(res, reg.cmd[i]);
        strcat(res, " ");
    }
    return res;
}

// void destroy_registerArray(struct registerArray *regArray)
// {
// }




int main()
{
    struct reg reg = create_register(1, time(NULL), 0, (char *[]){"ls", "/tmp", NULL});
    char *str = register_to_string(reg);
    printf("%s\n", str);
    free(str);
    return 0;
}