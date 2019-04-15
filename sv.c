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
		strings = fopen("STRINGS","ab+");
		stocks = fopen("STOCKS","rb+");
		vendas = fopen("VENDAS","ab+");
		fseek(artigos,0,SEEK_END);
		nextCode = (ftell(artigos)/sizeof(article))+1;
	}
	else{
		artigos = fopen("ARTIGOS","wb+");
		strings = fopen("STRINGS","ab+");
		stocks = fopen("STOCKS","wb+");
		vendas = fopen("VENDAS","ab+");		
	}

	int n;
	while((n=read(pipe,&q,sizeof(q)))>0){
		switch(q.operation){
			case 0:; // Connection establishment
				user element;
				element.pid = q.pid;
				strcpy(element.namedPipe,q.name);
				element.type = q.type;
				element.fd = open(q.name,O_RDWR);
				if(element.fd==-1) kill(q.pid,SIGINT);
				else userList = add(userList,element);
			break;
			case 1:; // Inserting a new article
				article articleToI;
				articleToI.price = q.value;
				articleToI.accesses=0;

				fseek(strings,0,SEEK_END);
				articleToI.ref = ftell(strings);
				fwrite(q.name,1,strlen(q.name),strings);
				
				fseek(artigos,0,SEEK_END);
				fwrite(&articleToI,sizeof(articleToI),1,artigos);

				fseek(stocks,nextCode*sizeof(int),SEEK_SET);
				fwrite(&articleToI.accesses,sizeof(int),1,stocks);
				char rep[20];
				sprintf(rep,"%d",nextCode);
				write(getPipe(userList,q.pid),rep,sizeof(rep));
				nextCode++;
			break;
			case 2: // Name changing
				fseek(artigos,q.code*sizeof(struct article),SEEK_SET);
				int location = ftell(strings);
				fwrite(&location,sizeof(int),1,artigos);
				fwrite(q.name,1,strlen(q.name),strings);
			break;
			case 3: // Price changing
				fseek(artigos,q.code*sizeof(struct article)+sizeof(int),SEEK_SET);
				fwrite(&q.value,sizeof(int),1,artigos);
				int flag3 = 0;

				for(int it3 = 0 ; it3 < cachedArticles && !flag3 ; it3++) {
					if(cache[it3].code == q.code) {
						flag3 = 1;
						cache[it3].price = q.value;
						cache[it3].accesses++;
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
					fseek(artigos,q.code*sizeof(struct article)+2*sizeof(int),SEEK_SET);					
					int acc3;
					fread(&acc3,sizeof(int),1,artigos);
					acc3++;
					cached c;
					c.code = q.code;
					c.price = q.value;
					c.stock = stk3;
					c.accesses = acc3;
					if (cachedArticles==maxCache) { // Updating cache
						if (acc3 >= cache[0].accesses) cache[0] = c;
					}	
					else cache[cachedArticles++] = c;
					qsort (cache, maxCache, sizeof(cached), compare);
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
				if (stockLoc4 >= fileLimit4) flag4 = 1; // If code the User is looking for is Out of File

				if(!flag4){ // Check if the item is in file
					fread(&s.stock,sizeof(int),1,stocks);
					fseek(artigos,q.code*sizeof(struct article)+sizeof(int),SEEK_SET);
					fread(&s.price,sizeof(int),1,artigos);

					int accesses4;
					fread(&accesses4, sizeof(int), 1, artigos);
					accesses4++;

					fseek(artigos,-sizeof(int),SEEK_CUR);
					fwrite(&accesses4,sizeof(int),1,artigos);
					
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
					s.stock = -1;
					s.price = -1;
					write(getPipe(userList,q.pid),&s,sizeof(stockAndPrice));
				}
				puts(" ");				
			break;
			case 5:; // Stock movement
				int flag5 = 0;
				int stock;
				int price;
				sale s5;
				s5.code = q.code;
				s5.quantity = q.value;
				for(int it4 = 0 ; it4 < cachedArticles ; it4++) { // Check if the item is in cache
					if(cache[it4].code==q.code) {
						flag5 = 1;
						cache[it4].stock -= (-1*q.value);
						cache[it4].accesses++;
						write(getPipe(userList,q.pid),&cache[it4].stock,sizeof(int));
						s5.paidAmount = cache[it4].price*q.value;

						fseek(vendas,0,SEEK_END);
						fwrite(&s5,sizeof(sale),1,vendas);
						break;
					}
				}

				if (flag5) break;

				fseek(stocks,0,SEEK_END);
				int fileLimit5 = ftell(stocks);
				fseek(stocks,q.code*sizeof(int),SEEK_SET); 
				int stockLoc5 = ftell(stocks);
				if (stockLoc5 >= fileLimit5) flag5 = 1;

				if(!flag5){ // Check if the item is in the file
					fread(&stock,sizeof(int),1,stocks);
					stock -= (-1*q.value);

					fseek(artigos,q.code*sizeof(struct article)+sizeof(int),SEEK_SET);
					fread(&price,sizeof(int),1,artigos);

					s5.paidAmount = price*(-1*q.value);
				
					fseek(vendas,0,SEEK_END);
					fwrite(&s5,sizeof(sale),1,vendas);

					//printf("SALE: Code %d quantity %d paidAmount %d\n",s5.code,s5.quantity,s5.paidAmount);

					int acc5;
					fread(&acc5, sizeof(int), 1, artigos);
					acc5++;

					fseek(artigos,-sizeof(int),SEEK_CUR);
					fwrite(&acc5,sizeof(int),1,artigos);
					//printf("Item with code %d has %d accesses\n", q.code,acc5);
					
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

   							//printf("repl.accesses %d repl.code %d\n",repl.accesses,repl.code);
   							fseek(stocks,repl.code*sizeof(int),SEEK_SET);
   							//printf("Replaced written to %d\n",(int) ftell(stocks));  							
   							fwrite(&repl.stock,sizeof(int),1,stocks);
   							//printf("CASE5: Wrote %d to stocks\n",repl.stock);
						}
					}	
					else {
						//printf("Added to cache article %d with %d stock\n",c.code,c.stock);
						cache[cachedArticles++] = c; 
					}
					qsort (cache, cachedArticles, sizeof(cached), compare);					
				}
				else{ // Item doesn't exist
					printf("Item with code %d DOES NOT EXIST\n",q.code);
					s.stock = -1;
					s.price = -1;
					write(getPipe(userList,q.pid),&s,sizeof(stockAndPrice));
				}				
				puts(" ");				
			break;
			case 6: // User disconnecting
				kill(q.pid, SIGUSR1);
				close(getPipe(userList,q.pid));
				removeN(userList,q.pid);
				puts("");
			break;
		}
		//sleep(1);
	}

	for (int k = 0 ; k < cachedArticles ; k++) {
		fseek(artigos,cache[k].code*sizeof(struct article)+2*sizeof(int),SEEK_SET);
		fwrite(&cache[k].accesses,sizeof(int),1,artigos);
		fseek(stocks,cache[k].code*sizeof(int),SEEK_SET);
		fwrite(&cache[k].stock,sizeof(int),1,stocks);
	}

	fseek(artigos,0,SEEK_SET);
	struct article *a2 = malloc(sizeof(struct article));
	for (int k = 0;fread(a2,sizeof(struct article),1,artigos);k++){
		int stock;
		fseek(stocks,k*sizeof(int),SEEK_SET);
		fread(&stock,sizeof(int),1,stocks);
		printf("CODE %d: a1.ref=%d\ta1.price=%d\ta1.accesses=%d\tand final stock %d\n",k,a2->ref,a2->price,a2->accesses,stock);
	}

	/*
	struct sale sv;
	fseek(vendas,0,SEEK_SET);
	while(fread(&sv,sizeof(sale),1,vendas)>0) printf("Code: %d Amount %d Paid Amount %d\n",sv.code,sv.quantity,sv.paidAmount);
	*/
	
	close(pipe);
	
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