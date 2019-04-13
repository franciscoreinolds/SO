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

void init(NODE** head);
NODE* add(NODE* node, user toI);
void print_list(NODE* head);
void remove_node(NODE* head);
void removeN(NODE* head, int pid);
NODE *free_list(NODE *head);
int getPipe(NODE*head, int pid);
#endif