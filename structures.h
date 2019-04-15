#ifndef Structures_h
#define Structures_h

typedef struct query {
	int pid;
	int type;
	int operation;
	int code;
	int value;
	char name[128];
} query;

typedef struct reply {
	int code;
	int amount;
	int price;
}reply;

typedef struct article {
	int ref;
	int price;
	int accesses;
}article;

typedef struct stock {
	int amount;
}stock;

typedef struct string {
	char* name;
}name;

typedef struct sale {
	int code;
	int quantity;
	int paidAmount;
}sale;

typedef struct user {
	int pid;
	char namedPipe[128];
	int type;
	int fd;
}user;

typedef struct node {
    user data;
    struct node* next;
} NODE;

typedef struct cached{
	int code;
	int price;
	int stock;
	int accesses;
}cached;

typedef struct stockAndPrice{
	int stock;
	int price;
}stockAndPrice;

void init(NODE** head);
NODE* add(NODE* node, user toI);
void print_list(NODE* head);
void remove_node(NODE* head);
void removeN(NODE* head, int pid);
NODE *free_list(NODE *head);
int getPipe(NODE*head, int pid);


#define StackItem struct article

typedef struct Stack Stack;

Stack *stackCreate();
void stackDestroy(Stack *stack);
void stackClean(Stack *stack);
int stackIsEmpty(Stack *stack);
size_t stackSize(Stack *stack);
StackItem stackTop(Stack *stack);
int stackPush(Stack *stack, StackItem item);
StackItem stackPop(Stack *stack);
StackItem getItem(Stack *stack, int code);

#endif