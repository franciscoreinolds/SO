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

#define maxCache 5

int cachedArticles = 0;
int clientN = 0;
int nextCode = 0;
int stringsSize = 0;
int waste = 0;
int childAmount = 0;
int maxCap = 20*sizeof(sale);
cached cache[maxCache];
NODE* userList;
query q;
int strings;
int artigos;
int stocks;
int vendas;

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
	mkfifo("pipe",0666);
	int pipe = open("pipe",O_RDONLY);
	int n;
	while((n=read(pipe,&q,sizeof(q)))>0){
		int fileLimit = (int) lseek(stocks,0,SEEK_END); 
		int stockLoc = (int) lseek(stocks,q.code*sizeof(int),SEEK_SET);
		switch(q.operation){
			case 0:; // Connection establishment
				user element;
				element.pid = q.pid;
				strcpy(element.namedPipe,q.name);
				element.type = q.type;
				element.fd = open(q.name,O_WRONLY);
				if(element.fd==-1) {
					while(element.fd==-1) element.fd = open(q.name,O_WRONLY);
				}
				userList = add(userList,element);
			break;
			case 1:; // Price changing
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
				}
			break;
			case 2:; // Stock checking
				stockAndPrice s;
				if (stockLoc < fileLimit) {

					int i = checkCache();
					if (i+1) {
						s.stock = cache[i].stock;
						s.price = cache[i].price;
						write(getPipe(userList,q.pid),&s,sizeof(stockAndPrice));
						break; // If article was found in cache 
					}
					else { // Check if the item is in file
						read(stocks,&s.stock,sizeof(int));
						lseek(artigos,q.code*sizeof(struct article)+2*sizeof(int),SEEK_SET);
						read(artigos,&s.price,sizeof(int));
						int acc2;
						read(artigos,&acc2, sizeof(int));
						acc2++;

						lseek(artigos,-sizeof(int),SEEK_CUR);
						write(artigos,&acc2,sizeof(int));
						
						updateCache(s.price, s.stock, acc2, 2);

						write(getPipe(userList,q.pid),&s,sizeof(stockAndPrice));					
					}
				}
				else{ // Item doesn't exist
					s.stock = -1;
					s.price = -1;
					write(getPipe(userList,q.pid),&s,sizeof(stockAndPrice));
				}
			break;
			case 3:; // Stock movement
				if (stockLoc < fileLimit) {
					int stock;
					int price;
					sale s5;
					s5.code = q.code;
					s5.quantity = q.value;

					int i = checkCache();
					if (i+1) {
						cache[i].stock += (-1*q.value);
						write(getPipe(userList,q.pid),&cache[i].stock,sizeof(int));
						s5.paidAmount = cache[i].price*q.value;
						lseek(vendas,0,SEEK_END);
						if (q.value>0) write(vendas,&s5,sizeof(sale));
						break;
					}
					else { // Check if the item is in the file
						read(stocks,&stock,sizeof(int));
						stock += (-1*q.value);
						lseek(stocks,-sizeof(int),SEEK_CUR);
						write(stocks,&stock,sizeof(int));

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
						
						write(getPipe(userList,q.pid),&stock,sizeof(int));
						
						updateCache(price, stock, acc3, 3);				
					}
				}	
				else{ // Item doesn't exist
					int tos = -1;
					write(getPipe(userList,q.pid),&tos,sizeof(int));
				}
			break;
			case 4: // User disconnecting
				printf("Gonna kill %d\n",q.pid);
				kill(q.pid, SIGUSR2);
				close(getPipe(userList,q.pid));
				removeN(userList,q.pid);
			break;
		}
	}

	cacheSaving();
		
	/*
	struct sale sv;
	lseek(vendas,0,SEEK_SET);
	while(read(vendas,&sv,sizeof(sale))>0) printf("Code: %d Amount %d Paid Amount %d\n",sv.code,sv.quantity,sv.paidAmount);
	*/
	
	close(pipe);
	
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