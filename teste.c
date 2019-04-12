#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h> 
#include <sys/types.h> 

struct article {
	int ref;
	int price;
	int accesses;
};

struct stock {
	int amount;
};

struct strings {
	char* nome;
};

struct sale {
	int code;
	int quantity;
	int payment;
};

int main(int argc, char const *argv[]) {
	
	/*
	FILE* teste = fopen("test","wb+");
	
	for (int i=0 ;  i<10 ; i++) {
		struct article *a1 = malloc(sizeof(struct article));
		a1->ref = 0;
		a1->price = i+1;
		a1->accesses = 0;
		fwrite(a1,sizeof(struct article),1,teste);
	}

	fclose(teste);
	*/

	FILE* coming = fopen("test","r");

	struct article *a2 = malloc(sizeof(struct article));

	fseek(coming,3*sizeof(struct article),SEEK_SET);

	fread(a2,sizeof(struct article),1,coming);

	printf("a1.ref=%d, a1.price=%d, a1.accesses=%d\n",a2->ref,a2->price,a2->accesses);

	printf("sizeof(struct article): %ld\n", sizeof(struct article));
	printf("sizeof(struct stock): %ld\n", sizeof(struct stock));
	printf("sizeof(struct strings): %ld\n", sizeof(struct strings));
	printf("sizeof(struct sale): %ld\n", sizeof(struct sale));
	
	fclose(coming);
	
	return 0;
}