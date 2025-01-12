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
	int refI;
	int refF;
	int price;
	int accesses;
}article;

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

typedef struct stockAndPrice{
	int stock;
	int price;
}stockAndPrice;

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

void init(NODE** head);
NODE* add(NODE* node, user toI);
NODE* removeN(NODE* head, int pid);
NODE *free_list(NODE *head);
int getPipe(NODE*head, int pid);
void print_list(NODE* head);
int sizeList(NODE* head);
int pop(NODE* head);
void print_sale(sale s, int i);
int getLine(int fd, char* buffer, int n);
int space_counter(char* buf);

#endif