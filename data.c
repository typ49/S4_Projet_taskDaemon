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

struct reg copy_reg(const struct reg *source) {
    struct reg copy;
    copy.num_cmd = source->num_cmd;
    copy.start = source->start;
    copy.period = source->period;

    size_t cmd_count = 0;
    while (source->cmd[cmd_count] != NULL) {
        cmd_count++;
    }

    copy.cmd = malloc((cmd_count + 1) * sizeof(char *));
    if (copy.cmd == NULL) {
        perror("copy_reg malloc");
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < cmd_count; ++i) {
        copy.cmd[i] = strdup(source->cmd[i]);
        if (copy.cmd[i] == NULL) {
            perror("copy_reg strdup");
            exit(EXIT_FAILURE);
        }
    }

    copy.cmd[cmd_count] = NULL;
    return copy;
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
    // 4. size of the ';' , '\n' and '\0'
    len += 5;

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
    strcat(res, "\n");
    return res;
}



struct registerArray create_registerArray(size_t capacity)
{
    struct registerArray regArray;
    regArray.size = 0;
    regArray.capacity = capacity;
    regArray.array = malloc(capacity*sizeof(struct reg));
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
            char **command = regArray->array[i].cmd;
            for(size_t j = 0; command[j] != NULL; ++j) {
                free(command[j]);
            }
            free(command);
            regArray->size--;
            return;
        }
    }
}

void destroy_registerArray(struct registerArray *regArray)
{
    for (size_t i = 0; i < regArray->size; ++i)
    {
        char **command = regArray->array[i].cmd;
        for(size_t j = 0; command[j] != NULL; ++j) {
            free(command[j]);
        }
        free(command);
    }
    free(regArray->array);
    regArray->size = 0;
    regArray->capacity = 0;
}
