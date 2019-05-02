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

    NODE* toInsert = (NODE*) malloc(sizeof (NODE));
    toInsert->data = toI;
    toInsert->next = NULL;

    if (!toInsert) exit(0);

    if (!node) node = toInsert;

    else {
        NODE* temp = node;
        while (temp->next) temp = temp->next;
        temp->next = toInsert;
    }

    return node;
}

NODE* removeN(NODE* head, int pid){
    NODE* temp;
    NODE* prev;
    
    temp = head;

    if (temp == NULL) {
        exit(EXIT_FAILURE); // no memory available
    }

    if (temp && temp->data.pid == pid) { 
        unlink(temp->data.namedPipe);
        head = head->next;
        free(temp);
        temp = head;
        return head;
    } 

    puts("Nem chega aqui...");
    while (temp != NULL && temp->data.pid != pid) { 
        prev = temp; 
        temp = temp->next; 
    } 
  
    if (temp == NULL) return head; 

    unlink(temp->data.namedPipe);

    prev->next = temp->next; 
  
    free(temp);

    return head;
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

void print_list(NODE* head) {
    NODE * temp;
    for (temp = head; temp; temp = temp->next) printf("pid: %d\n",temp->data.pid);
    
}

int getPipe (NODE* head, int pid) {
    NODE * temp;
    for (temp = head; temp; temp = temp->next) if (temp->data.pid == pid) return temp->data.fd;
    return -1;
}

int sizeList(NODE* head) {
    int i = 0;
    NODE * temp;
    for (temp = head; temp; temp = temp->next) i++;
    return i;
}

int pop(NODE* head) {
    NODE* temp;
    temp = head;
    return temp->data.pid;
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