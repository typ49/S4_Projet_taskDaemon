

struct register {
    size_t num_cmd;
    size_t start;
    size_t period;
    char *cmd[];
}

/**
 * Create a register with the given parameters
 * 
 * @param num_cmd Number of the command
 * @param start Start time
 * @param period Period in seconds between two executions
 * @param cmd[] The command to execute (with its arguments) last argument must be NULL
 * 
 * @return The register
*/
struct register create_register(size_t num_cmd, size_t start, size_t period, char *cmd[]);


/**
 * Get the register to the format : num_cmd;start;period;cmd
 * 
 * With start beeing the lettre version of the start time for exemple :
 * 
 * 3;18 avril 10:44:57;0;ls /tmp/tasks
 * 
 * @param reg The register
 * 
 * @return The command to execute
*/
char *register_to_string(struct register reg);


struct registerArray {
    size_t size;
    struct register *array[];
}

/**
 * Create a register array with the given size
 * 
 * @param size The size of the array
 * 
 * @return The register array
*/
struct registerArray create_registerArray(size_t size);


/**
 * Add a register to the register array and take care of the size
 * 
 * @param regArray The register array
 * @param reg The register to add
 * 
 * @return void
*/
void add_register(struct registerArray *regArray, struct register reg);


/**
 * Remove a register from the register array and take care of the size
 * 
 * @param regArray The register array
 * @param num_cmd The number of the command to remove
 * 
 * @return void
*/
void suppress_register(struct registerArray *regArray, size_t num_cmd);
