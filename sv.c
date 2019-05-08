#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h> 
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include "structures.h"

#define maxCache 3

int cachedArticles = 0;
int childAmount = 0;
int clientN = 0;
int maxCap = 20*sizeof(sale);
int curMA = -1;
int nextCode = 0;
int stringsSize = 0;
int waste = 0;
cached cache[maxCache];
NODE* maList;
NODE* userList;
query q;
int strings;
int artigos;
int stocks;
int vendas;

void articleReader(){
	int lim = (int) lseek(artigos,0,SEEK_END);
	lseek(artigos,0,SEEK_SET);
	struct article a2;
	printf("lim: %d article %d\n",lim, (int) sizeof(article));
	for (int k = 0;read(artigos,&a2,sizeof(article));k++){
		int stock;
		lseek(stocks,k*sizeof(int),SEEK_SET);
		read(stocks,&stock,sizeof(int));
		printf("CODE %d: a1.refI=%d\ta1.refF=%d\ta1.price=%d\ta1.accesses=%d\tand final stock %d\n",k,a2.refI,a2.refF,a2.price,a2.accesses,stock);
		char name[1+a2.refF-a2.refI];
		lseek(strings,a2.refI,SEEK_SET);
		read(strings,&name,sizeof(char)*(a2.refF-a2.refI+1));
		name[a2.refF-a2.refI+1] = '\0';
		printf("CODE %d: NAME: %s\n",k,name);
		puts(" ");
	}	
}

int compare (const void * a, const void * b){
	const cached *a1 = (const cached *)a;
	const cached *b1 = (const cached *)b;
	return (a1->accesses - b1->accesses);
}

void cacheSaving(){
	for (int k = 0 ; k < cachedArticles ; k++) {
		lseek(artigos,cache[k].code*sizeof(struct article)+3*sizeof(int),SEEK_SET);
		write(artigos,&cache[k].accesses,sizeof(int));
		lseek(stocks,cache[k].code*sizeof(int),SEEK_SET);
		write(stocks,&cache[k].stock,sizeof(int));
	}
}

int checkCache(){
	for(int i = 0 ; i < cachedArticles; i++) {
		if(cache[i].code == q.code) {
			cache[i].accesses++;
			qsort (cache, cachedArticles, sizeof(cached), compare);
			return i;								
		}
	}
	return -1;
}

void updateCache(int price, int stk, int acc, int n){
	cached c;
	c.code = q.code;
	c.price = price;
	c.stock = stk;
	c.accesses = acc;
	if (cachedArticles == maxCache) { // Updating cache
		printf("acc%d: %d cache[0].accesses: %d\n", n, acc, cache[0].accesses);
		if (acc >= cache[0].accesses) {
			cached repl = cache[0];
			cache[0] = c;
			lseek(artigos,repl.code*sizeof(struct article)+3*sizeof(int),SEEK_SET);
			write(artigos,&repl.accesses,sizeof(int));
			lseek(stocks,repl.code*sizeof(int),SEEK_SET);
			write(stocks,&repl.stock,sizeof(int));
			printf("The item with code %d was replaced and its accesses, %d and stock %d got wrote to the respective file.\n",repl.code,repl.accesses,repl.stock);
		}
	}	
	else cache[cachedArticles++] = c;
	printf("Added item with code %d and price %d\n",c.code,c.price);
	qsort (cache, cachedArticles, sizeof(cached), compare);
	puts("Saiu");
}


int main(int argc, char const *argv[]){
	clock_t CPU_time_1 = clock();
	if(access("ARTIGOS",F_OK)!= -1) {
		artigos = open("ARTIGOS", O_RDWR);
		strings = open("STRINGS", O_APPEND | O_RDWR);
		stocks = open("STOCKS", O_RDWR);
		vendas = open("VENDAS", O_APPEND | O_RDWR);
		nextCode = (lseek(artigos,0,SEEK_END)/sizeof(article))+1;
	}
	else{
		artigos = open("ARTIGOS", O_CREAT | O_RDWR , 0644);
		strings = open("STRINGS", O_APPEND | O_CREAT | O_RDWR , 0644);
		stocks = open("STOCKS",O_CREAT | O_RDWR , 0644);
		vendas = open("VENDAS",O_APPEND | O_CREAT | O_RDWR , 0644);		
	}
	init(&userList);
	init(&maList);
	mkfifo("pipe",0666);
	int namedPipe = open("pipe",O_RDONLY);
	int n, counter = 0;
	while((n=read(namedPipe,&q,sizeof(q)))>0) {
		int fileLimit = (int) lseek(stocks,0,SEEK_END); 
		int stockLoc = (int) lseek(stocks,q.code*sizeof(int),SEEK_SET);
		switch(q.operation){
			case 0:; // Connection establishment
				user element;
				element.pid = q.pid;
				strcpy(element.namedPipe,q.name);
				element.type = q.type;
				element.fd = open(q.name,O_WRONLY);
				if (!element.type) { // is MA
					if (curMA==-1) {
						curMA = q.pid;
						kill(q.pid,SIGUSR2);
					}
					maList = add(maList,element);
					print_list(maList);
				}
				else {
					userList = add(userList,element);
					print_list(userList);
				}
			break;
			case 1:
				close(strings);
				strings = open("STRINGS", O_APPEND | O_CREAT | O_RDWR , 0644);
			break;
			case 2:; // Price changing
				if (stockLoc < fileLimit){
					lseek(artigos,q.code*sizeof(struct article)+2*sizeof(int),SEEK_SET);
					write(artigos,&q.value,sizeof(int));

					int i = checkCache();
					if (i+1){
						cache[i].price = q.value;
						break; // Article was found in cache 
					}
					else {
						int stk1, acc1;
						read(stocks,&stk1,sizeof(int));
					
						lseek(artigos,q.code*sizeof(struct article)+3*sizeof(int),SEEK_SET);					
						read(artigos,&acc1,sizeof(int));
						acc1++;
						lseek(artigos,-sizeof(int),SEEK_CUR);
						write(artigos,&acc1,sizeof(int));

						updateCache(q.value, stk1, acc1, 1);
					}
					counter++;
					if (counter==100) {
						cacheSaving(); 
						counter = 0;
					}
				}
			break;
			case 3:
				cacheSaving();
				close(vendas);
				rename("VENDAS","VENDASAG");
				vendas = open("VENDAS",O_APPEND | O_CREAT | O_RDWR , 0644);
				
				int des_p[2];
				int pipeOpening = pipe(des_p);
				if(pipeOpening == -1) exit(1);

				if(fork() == 0) {            //first fork
					close(STDOUT_FILENO);  //closing stdout
					dup(des_p[1]);         //replacing stdout with pipe write 
					close(des_p[0]);       //closing pipe read
					close(des_p[1]);

					char* prog1[3];
					prog1[0] = "cat";
					prog1[1] = "VENDASAG";
					prog1[2] =  0;
					execvp(prog1[0], prog1);
					exit(1);
				}

				if(fork() == 0) {            //creating 2nd child
					close(STDIN_FILENO);   //closing stdin
					dup(des_p[0]);         //replacing stdin with pipe read
					close(des_p[1]);       //closing pipe write
					close(des_p[0]);
					char* prog2[2];
					prog2[0] = "./ag";
					prog2[1] =  0;
					execvp(prog2[0], prog2);
					exit(1);
				}

				close(des_p[0]);
				close(des_p[1]);
				//sleep(1);
				/*
				wait(0);
				wait(0);
				*/
			break;
			case 4:; // Stock checking
				reply r2;
				stockAndPrice s;
				if (stockLoc < fileLimit) {
					r2.code = 0;
					int i = checkCache();
					if (i+1) {
						s.stock = cache[i].stock;
						r2.amount = cache[i].stock;
						s.price = cache[i].price;
						r2.price = cache[i].price;
						write(getPipe(userList,q.pid),&r2,sizeof(struct reply));
					}
					else { // Check if the item is in file
						read(stocks,&s.stock,sizeof(int));
						lseek(artigos,q.code*sizeof(struct article)+2*sizeof(int),SEEK_SET);
						read(artigos,&s.price,sizeof(int));
						int acc2;
						read(artigos,&acc2, sizeof(int));
						acc2++;

						r2.amount = s.stock;
						r2.price = s.price;

						lseek(artigos,-sizeof(int),SEEK_CUR);
						write(artigos,&acc2,sizeof(int));
						updateCache(s.price, s.stock, acc2, 2);
						write(getPipe(userList,q.pid),&r2,sizeof(struct reply));
					}
					counter++;
					if (counter==100) {
						printf("saved to cache!\n");
						cacheSaving(); 
						counter = 0;
					}
				}
				else{ // Item doesn't exist
					r2.code = -1;
					r2.amount = 0;
					r2.price = 0;
					write(getPipe(userList,q.pid),&r2,sizeof(reply));
				}
			break;
			case 5:; // Stock movement
				reply r3;
				if (stockLoc < fileLimit) {
					int stock;
					int price;

					r3.code = 1; // Indicating that it's a reply to CV's stock movement request 
					r3.price = 0;

					sale s5;
					s5.code = q.code;
					s5.quantity = q.value;


					int i = checkCache();
					if (i+1) {
						cache[i].stock += (-1*q.value);
						r3.amount = cache[i].stock;
						write(getPipe(userList,q.pid),&r3,sizeof(reply));
						s5.paidAmount = cache[i].price*q.value;
						lseek(vendas,0,SEEK_END);
						if (q.value>0) write(vendas,&s5,sizeof(sale));
					}
					else { // Check if the item is in the file
						read(stocks,&stock,sizeof(int));
						stock += (-1*q.value);
						lseek(stocks,-sizeof(int),SEEK_CUR);
						write(stocks,&stock,sizeof(int));
						r3.amount = stock;
						write(getPipe(userList,q.pid),&r3,sizeof(reply));
						
						lseek(artigos,q.code*sizeof(struct article)+2*sizeof(int),SEEK_SET);
						read(artigos,&price,sizeof(int));
						s5.paidAmount = price*q.value;
					
						lseek(vendas,0,SEEK_END);
						if (q.value>0) write(vendas,&s5,sizeof(sale));
						int acc3;
						read(artigos,&acc3, sizeof(int));
						acc3++;

						lseek(artigos,-sizeof(int),SEEK_CUR);
						write(artigos,&acc3,sizeof(int));

						updateCache(price, stock, acc3, 3);				
					}
					counter++;
					if (counter==100) {
						cacheSaving(); 
						counter = 0;
					}
				}	
				else{ // Item doesn't exist
					r3.amount = -1;
					r3.code = -1; // Indicates that the stock movement was unsuccessful due to the article not existing
					r3.price = -1;
					write(getPipe(userList,q.pid),&r3,sizeof(reply));
				}
			break;
			case 6: // User disconnecting
				if (!q.type){
					close(getPipe(maList,q.pid));
					maList = removeN(maList,q.pid);
					print_list(maList);
					puts(" ");
					if (maList) kill(pop(maList),SIGUSR2);		
					kill(q.pid, SIGUSR2);		
				}
				else {
					close(getPipe(userList,q.pid));
					userList = removeN(userList,q.pid);
				}
			break;
		}
		//sleep(1);
		//usleep(100000);
	}	

	cacheSaving();
	
	printf("Oi\n");

	articleReader();

	/*
	struct sale sv;
	lseek(vendas,0,SEEK_SET);
	while(read(vendas,&sv,sizeof(sale))>0) printf("Code: %d Amount %d Paid Amount %d\n",sv.code,sv.quantity,sv.paidAmount);
	*/
	
	close(namedPipe);
	
	close(artigos);
	close(stocks);
	close(strings);
	close(vendas);
	unlink("pipe");
    
    clock_t CPU_time_2 = clock();
    clock_t time_3 = (double) CPU_time_2 - CPU_time_1;
    printf("CPU end time is : %ld (ms)\n", (time_3*1000)/CLOCKS_PER_SEC);

	return 0;
}