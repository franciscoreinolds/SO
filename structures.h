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