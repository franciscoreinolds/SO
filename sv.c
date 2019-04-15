#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h> 
#include <sys/types.h>
#include <signal.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include "structures.h"

#define maxCache 5

int cachedArticles = 0;
int clientN = 0;
int nextCode = 0;
cached cache[maxCache];
NODE* userList;
query q;
FILE* strings;
FILE* artigos;
FILE* stocks;
FILE* vendas;

int compare (const void * a, const void * b){
	const cached *a1 = (const cached *)a;
	const cached *b1 = (const cached *)b;
	return (b1->accesses - a1->accesses);
}

int main(int argc, char const *argv[]){
	clock_t CPU_time_1 = clock();
	init(&userList);
	mkfifo("pipe",0666);
	int pipe = open("pipe",O_RDONLY);

	if(access("ARTIGOS",F_OK)!= -1) {
		artigos = fopen("ARTIGOS","rb+");
		strings = fopen("STRINGS","rb+");
		stocks = fopen("STOCKS","rb+");
		vendas = fopen("VENDAS","rb+");
		fseek(artigos,0,SEEK_END);
		nextCode = (ftell(artigos)/sizeof(article))+1;
	}
	else{
		artigos = fopen("ARTIGOS","ab+");
		strings = fopen("STRINGS","ab+");
		stocks = fopen("STOCKS","ab+");
		vendas = fopen("VENDAS","ab+");		
	}

	puts(" ");
	int n;
	while((n=read(pipe,&q,sizeof(q)))>0){
		switch(q.operation){
			case 0:; // Connection establishment
				user element;
				element.pid = q.pid;
				strcpy(element.namedPipe,q.name);
				element.type = q.type;
				element.fd = open(q.name,O_RDWR);
				printf("ELEMENT FD: %d e %s\n",element.fd,q.name);
				if(element.fd==-1) kill(q.pid,SIGINT);
				else userList = add(userList,element);
			break;
			case 1: // Inserting a new article
				printf("The article to insert is called %s and costs %d\n",q.name,q.value);
				puts(" ");
				article articleToI;
				articleToI.price = q.value;
				articleToI.accesses=0;	
				articleToI.ref = ftell(strings);
				
				fseek(strings,articleToI.ref,SEEK_CUR);
				fwrite(q.name,1,strlen(q.name),strings);
				
				fwrite(&articleToI,sizeof(articleToI),1,artigos);

				fseek(stocks,nextCode*sizeof(int),SEEK_SET);
				fwrite(&articleToI.accesses,sizeof(int),1,stocks);
				
				char rep[20];
				sprintf(rep,"%d",nextCode);
				write(getPipe(userList,q.pid),rep,sizeof(rep));
				nextCode++;
			break;
			case 2: // Name changing
				printf("The article who will have it's name changed has as code %d and will be called %s\n",q.code,q.name);
				puts(" ");
				fseek(artigos,q.code*sizeof(struct article),SEEK_SET);
				int location = ftell(strings);
				fwrite(&location,sizeof(int),1,artigos);
				fwrite(q.name,1,strlen(q.name),strings);
			break;
			case 3: // Price changing
				printf("The article who will have it's price changed has as code  %d and will have it's price changed to %d\n",q.code,q.value);
				puts(" ");
				fseek(artigos,q.code*sizeof(struct article)+sizeof(int),SEEK_SET);
				int value = q.value;
				fwrite(&value,sizeof(int),1,artigos);
				int flag3 = 0;
				
				for(int it3 = 0 ; it3 < cachedArticles && !flag3 ; it3++) {
					if(cache[it3].code == q.code) {
						flag3 = 1;
						cache[it3].price = q.value;
						cache[it3].accesses++;
						//fwrite(&cache[it3].accesses,sizeof(int),1,artigos);
					}
				}

				if(flag3) break; // Article was found in cache 
				
				fseek(stocks,0,SEEK_END);
				int fileLimit3 = ftell(stocks);
				fseek(stocks,q.code*sizeof(int),SEEK_SET); 
				int stockLoc3 = ftell(stocks);
				if (stockLoc3 > fileLimit3) flag3 = 1; // If code the User is looking for is Out of File				
				
				int stk3;
				fread(&stk3,sizeof(int),1,stocks);
				
				if(!flag3) {
					int acc3;
					fread(&acc3,sizeof(int),1,artigos);
					acc3++;
					fseek(artigos,-sizeof(int),SEEK_CUR);
					fwrite(&acc3,sizeof(int),1,artigos);
					if (cachedArticles==maxCache) { // Updating cache
						if (acc3 >= cache[0].accesses) {
							cached c;
							c.code = q.code;
							c.price = value;
							c.stock = stk3;
							c.accesses = acc3;
							cache[0] = c;
							qsort (cache, cachedArticles, sizeof(cached), compare);
						}
					}	
					else {
						cached c;
						c.code = q.code;
						c.price = value;
						c.stock = stk3;
						c.accesses = acc3;
						cache[cachedArticles++] = c;						
						qsort (cache, maxCache, sizeof(cached), compare);
					}
				}
			break;
			case 4:; // Stock checking
				stockAndPrice s;
				int flag4 = 0;
				for(int it4 = 0 ; it4 < cachedArticles ; it4++) { // Check if the item is in cache
					if(cache[it4].code==q.code) {
						flag4 = 1;
						s.stock = cache[it4].stock;
						s.price = cache[it4].price;
						cache[it4].accesses++;
						printf("CASE4 FOUND it %d artigo tem code %d e %d accesses\n",it4,cache[it4].code,cache[it4].accesses);						
						write(getPipe(userList,q.pid),&s,sizeof(stockAndPrice));
   						qsort (cache, cachedArticles, sizeof(cached), compare);
						break;
					}
				}

				if(flag4) break; // If article was found in cache

				fseek(stocks,0,SEEK_END);
				int fileLimit4 = ftell(stocks);
				fseek(stocks,q.code*sizeof(int),SEEK_SET); 
				int stockLoc4 = ftell(stocks);
				printf("stockLoc4 %d fileLimit4 %d\n", stockLoc4,fileLimit4);
				if (stockLoc4 >= fileLimit4) flag4 = 1; // If code the User is looking for is Out of File

				if(!flag4){ // Check if the item is in file
					printf("CASE4: Checking if the item with code %d is in the file\n",q.code);	

					fread(&s.stock,sizeof(int),1,stocks);
					fseek(artigos,q.code*sizeof(struct article)+sizeof(int),SEEK_SET);
					fread(&s.price,sizeof(int),1,artigos);

					int accesses4;
					fread(&accesses4, sizeof(int), 1, artigos);
					printf("price: %d, accesses4 before increment: %d\n",s.price, accesses4);
					accesses4++;

					fseek(artigos,-sizeof(int),SEEK_CUR);
					fwrite(&accesses4,sizeof(int),1,artigos);
					printf("Item with code %d has %d accesses\n", q.code,accesses4);
					
					if (cachedArticles==maxCache) { // Updating cache
						if (accesses4 >= cache[0].accesses) {
							cached c;
							c.code = q.code;
							c.price = s.price;
							c.stock = s.stock;
							c.accesses = accesses4;
							cached repl = cache[0];
							cache[0] = c;
   							qsort (cache, cachedArticles, sizeof(cached), compare);
   							fseek(artigos,repl.code*sizeof(struct article)+2*sizeof(int),SEEK_SET);
   							fwrite(&repl.accesses,sizeof(int),1,artigos);
   							fseek(stocks,repl.code*sizeof(int),SEEK_SET);
   							fwrite(&repl.stock,sizeof(int),1,stocks);
						}
					}	
					else {
						cached c;
						c.code = q.code;
						c.price = s.price;
						c.stock = s.stock;
						c.accesses = accesses4;
						cache[cachedArticles++] = c;						
   						qsort (cache, maxCache, sizeof(cached), compare);
					}
					write(getPipe(userList,q.pid),&s,sizeof(stockAndPrice));					
				}
				else{ // Item doesn't exist
					printf("Item with code %d DOES NOT EXIST\n",q.code);
					s.stock = -1;
					s.price = -1;
					write(getPipe(userList,q.pid),&s,sizeof(stockAndPrice));
					printf("Enviou para %d\n",getPipe(userList,q.pid));
				}
				puts(" ");				
			break;
			case 5:; // Stock movement
				printf("The client with pid %d wants to move %d units of article %d\n",q.pid,q.value,q.code);
				int flag5 = 0;
				int paidAmount = 0;
				int stock;
				int price;

				for(int it4 = 0 ; it4 < cachedArticles ; it4++) { // Check if the item is in cache
					if(cache[it4].code==q.code) {
						flag5 = 1;
						cache[it4].stock -= (-1*q.value);
						paidAmount += (cache[it4].price*(-1*q.value));
						cache[it4].accesses++;
						write(getPipe(userList,q.pid),&cache[it4].stock,sizeof(int));
						sale s;
						s.code = q.code;
						s.quantity = q.value;
						s.paidAmount = cache[it4].price*q.value;
						printf("Sale: Code %d quantity %d paidAmount %d\n",s.code,s.quantity,s.paidAmount);
						printf("CASE 5: Artigo de code %d tem %d accesses\n",cache[it4].code,cache[it4].accesses);
						break;
					}
				}

				printf("Oi\n");

				fseek(stocks,0,SEEK_END);
				int fileLimit5 = ftell(stocks);
				fseek(stocks,q.code*sizeof(int),SEEK_SET); 
				int stockLoc5 = ftell(stocks);
				printf("stockLoc5 %d fileLimit5 %d\n", stockLoc5,fileLimit5);
				if (stockLoc5 >= fileLimit5) flag5 = 1;

				printf("Breakou?\n");

				if(!flag5){ // Check if the item is in the file
					fread(&stock,sizeof(int),1,stocks);
					
					fseek(artigos,q.code*sizeof(struct article)+sizeof(int),SEEK_SET);
					fread(&price,sizeof(int),1,artigos);

					int acc5;
					fread(&acc5, sizeof(int), 1, artigos);
					printf("price: %d, accesses4 before increment: %d\n",price, acc5);
					acc5++;

					fseek(artigos,-sizeof(int),SEEK_CUR);
					fwrite(&acc5,sizeof(int),1,artigos);
					printf("Item with code %d has %d accesses\n", q.code,acc5);
					
					write(getPipe(userList,q.pid),&stock,sizeof(int));
					cached c;
					c.code = q.code;
					c.price = price;
					c.stock = stock;
					c.accesses = acc5;

					if (cachedArticles==maxCache) {
						if (acc5 >= cache[0].accesses) {
							cached repl = cache[0];
							cache[0] = c;

   							fseek(artigos,repl.code*sizeof(struct article)+2*sizeof(int),SEEK_SET);
   							fwrite(&repl.accesses,sizeof(int),1,artigos);

   							printf("repl.accesses %d repl.code %d\n",repl.accesses,repl.code);
   							fseek(stocks,repl.code*sizeof(int),SEEK_SET);
   							printf("Replaced written to %d\n",(int) ftell(stocks));  							
   							fwrite(&repl.stock,sizeof(int),1,stocks); 
						}
					}	
					else cache[cachedArticles++] = c; 
					qsort (cache, cachedArticles, sizeof(cached), compare);					
				}
				else{ // Item doesn't exist
					printf("Item with code %d DOES NOT EXIST\n",q.code);
					s.stock = -1;
					s.price = -1;
					write(getPipe(userList,q.pid),&s,sizeof(stockAndPrice));
				}				
				printf("Saiu\n");
				puts(" ");				
			break;
			case 6: // User disconnecting
				printf("Case 6\n");
				kill(q.pid, SIGUSR1);
				close(getPipe(userList,q.pid));
				removeN(userList,q.pid);
				puts("");
			break;
		}
		sleep(1);
	}

	for (int k = 0 ; k < cachedArticles ; k++) {
		printf("cache[%d].code: %d com %d accesses que custa %d\n", k , cache[k].code, cache[k].accesses, cache[k].price);		
		fseek(artigos,cache[k].code*sizeof(struct article)+2*sizeof(int),SEEK_SET);
		fwrite(&cache[k].accesses,sizeof(int),1,artigos);
		fseek(stocks,cache[k].code*sizeof(int),SEEK_SET);
		fwrite(&cache[k].stock,sizeof(int),1,stocks);
	}

	puts("------------------------------------");
	fseek(artigos,0,SEEK_SET);
	struct article *a2 = malloc(sizeof(struct article));
	while(fread(a2,sizeof(struct article),1,artigos)) printf("a1.ref=%d, a1.price=%d, a1.accesses=%d\n",a2->ref,a2->price,a2->accesses);
	close(pipe);
	puts(" ");
	
	fclose(artigos);
	fclose(stocks);
	fclose(strings);
	fclose(vendas);
	unlink("pipe");
 	userList = free_list(userList);	
    
    clock_t CPU_time_2 = clock();
    clock_t time_3 = (double) CPU_time_2 - CPU_time_1;
    printf("CPU end time is : %ld (s)\n", time_3/CLOCKS_PER_SEC);

	return 0;
}