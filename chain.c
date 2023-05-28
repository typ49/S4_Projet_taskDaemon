#include "chain.h"


struct chain *create_chain(){
    struct chain *new_chain = (struct chain*)malloc(sizeof(struct chain));
    new_chain->size = 0;
    new_chain->head = NULL;
    return new_chain;
}



void add_to_chain(struct chain* chain, size_t value) {
    struct node* new_node = (struct node*)malloc(sizeof(struct node));
    new_node->value = value;
    new_node->next = chain->head;
    chain->head = new_node;
}

void remove_from_chain(struct chain *chain) {
    struct node* temp = chain->head;
    chain->head = chain->head->next;
    free(temp);
}

void remove_from_chain_at_value(struct chain *chain, size_t value){
    struct node* temp = chain->head;
    struct node* prev = NULL;
    while(temp != NULL){
        if(temp->value == value){
            if(prev == NULL){
                chain->head = temp->next;
            }else{
                prev->next = temp->next;
            }
            free(temp);
            return;
        }
        prev = temp;
        temp = temp->next;
    }
}