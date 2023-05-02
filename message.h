// compilation: gcc -Wall -g -Wextra -std=c99 -o libmessage libmessage.c
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>



/**
 * Send a string to a file descriptor.
 * 
 * @param fd The file descriptor to write to.
 * @param str The string to write.
 * 
 * @return 0 on success, 1 on failure.
*/
int send_string(int fd, char *str);

/**
 * Receive a string from a file descriptor.
 * Be careful to free the string when you're done with it.
 * 
 * @param fd The file descriptor to read from.
 * 
 * @return The string read from the file descriptor.
 */
char *recv_string(int fd);

/**
 * Send an array of strings to a file descriptor.
 * 
 * @param fd The file descriptor to write to.
 * @param argv The array of strings to write.
 * 
 * @return 0 on success, 1 on failure.
 */
int send_argv(int fd, char *argv[]);


/**
 * Receive an array of strings from a file descriptor.
 * Be careful to free the array and its strings when you're done with it.
 * 
 * @param fd The file descriptor to read from.
 * 
 * @return The array of strings read from the file descriptor.
 */
char **recv_argv(int fd);