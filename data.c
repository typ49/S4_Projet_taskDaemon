#include "data.h"

struct register create_register(size_t num_cmd, size_t start, size_t period, char *cmd[]) {
    struct register reg;
    reg.num_cmd = num_cmd;
    reg.start = start;
    reg.period = period;
    reg.cmd = cmd;
    return reg;
}

char *register_to_string(struct register reg) {
    // calculate the size of the string
    // 1. size of the command
    size_t len = 0;
    for(size_t i = 0; reg.cmd[i] != NULL; ++i) {
        len += strlen(reg.cmd[i]);
    }
    // 2. size the num_cmd and period fields
    len += 2 * sizeof(size_t);
    // 3. size of the start fieldh
    len += 22; // a date with the maximum size beeing : '10 septembre 23:59:59\0'

    // create the string with calloc so no need to add the '\0' at the end
    char *res = calloc(len, sizeof(char));
}