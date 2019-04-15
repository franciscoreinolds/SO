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

typedef struct StackNode {
    StackItem item;             /** The data of this node. **/
    struct StackNode *next;     /** The next node (the one below the top). **/
} StackNode;

struct Stack {
    size_t count;   /** The number of items in the stack. **/
    StackNode *top; /** The top item of the stack. **/
};

Stack *stackCreate(){
    /* Create a stack and set everything to the default values. */
    Stack *stack = (Stack *) malloc(sizeof *stack);
    if(stack == NULL)
        return NULL;
    
    stack->count = 0;
    stack->top = NULL;
    
    return stack;
}

void stackDestroy(Stack *stack){
    stackClean(stack);
    free(stack);
}

void stackClean(Stack *stack){
    while(!stackIsEmpty(stack))
        stackPop(stack);
}

int stackIsEmpty(Stack *stack){
    return stack->top == NULL ? 1 : 0;
}

size_t stackSize(Stack *stack){
    return stack->count;
}

StackItem stackTop(Stack *stack){
    return stack->top->item;
}

int stackPush(Stack *stack, StackItem item){
    StackNode *newNode = (StackNode *) malloc(sizeof *newNode);
    if(newNode == NULL)
        return 0;
    
    newNode->item = item;
    newNode->next = stack->top;
    stack->top = newNode;
    
    stack->count += 1;
    return 1;
}

StackItem stackPop(Stack *stack){
    StackNode *oldTop;
    StackItem item;
    
    if(stack->top == NULL) {
        struct article a = {0};
        return a;
    }
    
    oldTop = stack->top;
    item = oldTop->item;
    stack->top = oldTop->next;
    free(oldTop);
    oldTop = NULL;
    
    stack->count -= 1;
    return item;
}

StackItem getItem(Stack *stack, int code){
    StackNode* tmp;
    StackItem item;
    
    if(stack->top == NULL) {
        printf("Stack top NULL\n");
        struct article a = {0};
        return a;
    }

    tmp = stack->top;
    item = tmp->item;

    while(item.ref!=code && tmp!=NULL) {
        printf("ref: %d, code %d\n",item.ref,code);
        tmp = tmp->next;
        item = tmp->item;
    }

    if (!tmp) {
        struct article a;
        a.price = -1;
        a.ref = -1;
        a.accesses = -1;
        return a;
    }
    
    return item;
}