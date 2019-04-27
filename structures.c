#include <stdio.h> 
#include <stdlib.h> 
#include <limits.h>
#include <unistd.h>
#include <string.h>
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

void removeN(NODE* head, int pid){
    NODE* temp;
    NODE* prev;
    
    temp = head;

    if (temp == NULL) {
        exit(EXIT_FAILURE); // no memory available
    }

    if (temp != NULL && temp->data.pid == pid) { 
        unlink(temp->data.namedPipe);
        head = temp->next;
        free(temp); 
        return; 
    } 

    while (temp != NULL && temp->data.pid != pid) { 
        prev = temp; 
        temp = temp->next; 
    } 
  
    if (temp == NULL) return; 

    unlink(temp->data.namedPipe);

    prev->next = temp->next; 
  
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

void print_sale(sale s, int i){
    printf("Sale %d: Code: %d\t Quantity %d\t PaidAmount\t%d\n",i,s.code,s.quantity,s.paidAmount);
}

int getLine(int fd, char* buffer, int n){
    char buf;
    int max = n;
    int cur = 0;
    while(read(fd,&buf,1) && cur < max && buf!='\n') buffer[cur++]=buf;
    buffer[cur] = '\0';
    return cur;
}

int space_counter(char* buf){
    int res = 0;
    for(int i = 0; buf[i]!='\0'; i++) if (buf[i]==' ') res++;
    return res;
}