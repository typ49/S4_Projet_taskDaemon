#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <locale.h>

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

    struct tm *tm;

    tm = localtime(&reg.start);

    // calculate the size of the string
    // 1. size of the command
    size_t len = 0;
    for (size_t i = 0; reg.cmd[i] != NULL; ++i)
    {
        len += strlen(reg.cmd[i]) + 1;
    }
    // 2. size the num_cmd and period fields
    len += 20;
    len += 20;
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

    // format the date to the french format
    strftime(start, 21, "%d %B %H:%M:%S", tm);
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

struct registerArray create_registerArray(size_t size)
{
    struct registerArray regArray;
    regArray.size = size;
    regArray.capacity = size;
    regArray.array = malloc(size*sizeof(struct reg));
    return regArray;
}

void add_register(struct registerArray *regArray, struct reg reg)
{
    if (regArray->size == regArray->capacity)
    {
        regArray->capacity *= 2;
        regArray->array = realloc(regArray->array, regArray->capacity * sizeof(struct reg));
    }
    regArray->array[regArray->size] = reg;
    regArray->size++;
}

void suppress_register(struct registerArray *regArray, size_t num_cmd)
{
    for (size_t i = 0; i < regArray->size; ++i)
    {
        if (regArray->array[i].num_cmd == num_cmd)
        {
            for (size_t j = i; j < regArray->size - 1; ++j)
            {
                regArray->array[j] = regArray->array[j + 1];
            }
            regArray->size--;
            return;
        }
    }
}

void destroy_registerArray(struct registerArray *regArray)
{
    for (size_t i = 0; i < regArray->size; ++i)
    {
        free(regArray->array[i].cmd);
    }
    regArray->size = 0;
    regArray->capacity = 0;
}

int main()
{
    setlocale(LC_ALL, "fr_FR.UTF-8");

    char *cmd[] = {"ls", "-l", NULL};
    struct reg reg = create_register(1, time(NULL), 0, cmd);
    char *str = register_to_string(reg);
    printf("%s\n", str);
    free(str);

    struct registerArray regArray = create_registerArray(1);
    add_register(&regArray, reg);
    destroy_registerArray(&regArray);
    return 0;
}

// gcc -Wall -Wextra -std=c99 -pedantic -o data data.c