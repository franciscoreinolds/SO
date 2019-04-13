#include <stdio.h> 
#include <stdlib.h> 
#include <limits.h> 
#include "structures.h"

void init(NODE** head) {
    *head = NULL;
}

NODE* add(NODE* node, user toI) {
    NODE* temp = (NODE*) malloc(sizeof (NODE));
    if (temp == NULL) {
        exit(0); // no memory available
    }
    temp->data = toI;
    temp->next = node;
    node = temp;
    return node;
}

void print_list(NODE* head) {
    NODE * temp;
    for (temp = head; temp; temp = temp->next)
        printf("Pid: %d\n", temp->data.pid);
}

void removeN(NODE* head, int pid){
    NODE* temp = (NODE*) malloc(sizeof (NODE));
    NODE* prev;
    
    temp = head;

    if (temp == NULL) {
        exit(EXIT_FAILURE); // no memory available
    }

    if (temp != NULL && temp->data.pid == pid) { 
        head = temp->next;
        free(temp); 
        return; 
    } 

    while (temp != NULL && temp->data.pid != pid) { 
        prev = temp; 
        temp = temp->next; 
    } 
  
    if (temp == NULL) return; 
  
    prev->next = temp->next; 
  
    free(temp);
}

void remove_node(NODE* head) {
    NODE* temp = (NODE*) malloc(sizeof (NODE));
    if (temp == NULL) {
        exit(EXIT_FAILURE); // no memory available
    }
    temp = head->next;
    head->next = head->next->next;
    free(temp);
}

NODE *free_list(NODE *head) {
    NODE *tmpPtr = head;
    NODE *followPtr;
    while (tmpPtr != NULL) {
        followPtr = tmpPtr;
        tmpPtr = tmpPtr->next;
        free(followPtr);
    }
    return NULL;
}

int getPipe (NODE* head, int pid) {
    NODE * temp;
    for (temp = head; temp; temp = temp->next) if (temp->data.pid == pid) return temp->data.fd;
    return -1;
}