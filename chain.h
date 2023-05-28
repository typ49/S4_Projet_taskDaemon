#include <stdlib.h>
#include <stdio.h>


struct node {
    size_t value;
    struct node *next;
};

struct chain {
    struct node *head;
    size_t size;
};

/**
 * @brief create a new chain
*/
struct chain *create_chain();

/**
 * @brief add a value to the beginning of the chain
*/
void add_to_chain(struct chain *chain, size_t value);

/**
 * @brief remove the first value from the chain
*/
void remove_from_chain(struct chain *chain);


/**
 * @brief remove the first value from the chain
*/
void remove_from_chain_at_value(struct chain *chain, size_t value);