#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

struct reg {
    size_t num_cmd;
    time_t start;
    size_t period;
    char **cmd;
};

/**
 * Create a register with the given parameters
 * 
 * @param num_cmd Number of the command
 * @param start Start time
 * @param period Period in seconds between two executions
 * @param cmd[] The command to execute (with its arguments) last argument must be NULL
 * 
 * @return The reg
*/
struct reg create_register(size_t num_cmd, time_t start, size_t period, char *cmd[]);


/**
 * Copy a register.
 * Be carefull to free the memory of its cmd
 * 
 * @param source The register to copy
 * 
 * @return The copy of the register
*/
struct reg copy_reg(const struct reg *source);

/**
 * Get the register to the format : num_cmd;start;period;cmd '\n'
 * 
 * With start beeing the lettre version of the start time for exemple :
 * 
 * 3;18 avril 10:44:57;0;ls /tmp/tasks
 * 
 * make sure to free the result
 * 
 * @param reg The register
 * 
 * @return The command to execute
*/
char *register_to_string(struct reg reg);


struct registerArray {
    size_t size;
    size_t capacity;
    struct reg *array;
};

/**
 * Create a register array with the given capacity
 * 
 * @param capacity The capacity of the array
 * 
 * @return The register array
*/
struct registerArray create_registerArray(size_t capacity);


/**
 * Add a register to the register array and take care of the size
 * 
 * @param regArray The register array
 * @param reg The register to add
 * 
 * @return void
*/
void add_register(struct registerArray *regArray, struct reg reg);


/**
 * Remove a register from the register array and take care of the size
 * 
 * @param regArray The register array
 * @param num_cmd The number of the command to remove
 * 
 * @return void
*/
void suppress_register(struct registerArray *regArray, size_t num_cmd);


/**
 * Destroy the register array and free the memory
 * 
 * @param regArray The register array to destroy
 * 
 * @return void
*/
void destroy_registerArray(struct registerArray *regArray);