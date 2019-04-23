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
int stringsSize = 0;
int waste = 0;
cached cache[maxCache];
NODE* userList;
query q;
int strings;
int artigos;
int stocks;
int vendas;

void childProcess(int i){
	char fileName[4];
	sprintf(fileName,"ag%d",i);

	int fd = open(fileName,O_CREAT | O_RDWR, 0644);
	int it, processedArticles = 0, pos = 0;
	int salesLimit = (int) lseek(vendas,0,SEEK_END); 

	for(it = 0; it < childAmount && pos < salesLimit ; it++){
		int curItem = i*1000000*sizeof(sale) + it*sizeof(sale);
		lseek(vendas,curItem,SEEK_SET);
		sale toP;
		read(vendas,&toP,sizeof(sale));
		if (toP.code==(curItem/sizeof(sale))) {
			lseek(fd,curItem+sizeof(int),SEEK_SET);
			int totalSales,totalAmount;
			read(fd,&totalSales,sizeof(int));
			read(fd,&totalAmount,sizeof(int));
			
			totalSales += toP.quantity;
			totalAmount += toP.paidAmount;
			lseek(fd,-2*sizeof(int),SEEK_CUR);
			write(fd,&totalSales,sizeof(int));
			write(fd,&totalAmount,sizeof(int));			
		}
	}
	close(fd);
	_exit(i);
}

void aggregator(int s){
	time_t rawtime;
	struct tm* timeinfo;
	time (&rawtime);
	timeinfo = localtime (&rawtime);
	char time[25];
	sprintf(time,"%d-%d-%dT%d:%d:%d",timeinfo->tm_year+1900,timeinfo->tm_mon+1,timeinfo->tm_mday,timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec);
	printf("time: %s\n",time);
	int saleLength = (int) lseek(vendas,0,SEEK_END);
	int childAmount = (saleLength / 1000000*sizeof(struct sale)) + 1;
	/*
	if (childAmount > 10) childAmount = 10;
	int pids[childAmount];
	for (int i = 0 ; i < childAmount ; i++){
		pids[i] = fork();
		switch(pids[i]){
			case -1:
				perror("Fork failure");
			break;
			case 0:
				childProcess(i);
			break;
		}
	} 

	for (int i = 0 ; i < childAmount ; i++) wait(NULL);
	*/
	printf("sl: %d ca: %d\n",saleLength,childAmount);
    signal(SIGINT, aggregator);
}

void articleReader(){
	lseek(artigos,0,SEEK_SET);
	struct article a2;
	for (int k = 0;read(artigos,&a2,sizeof(struct article));k++){
		int stock;
		lseek(stocks,k*sizeof(int),SEEK_SET);
		read(stocks,&stock,sizeof(int));
		printf("CODE %d: a1.refI=%d\ta1.refF=%d\ta1.price=%d\ta1.accesses=%d\tand final stock %d\n",k,a2.refI,a2.refF,a2.price,a2.accesses,stock);
		char name[1+a2.refF-a2.refI];
		lseek(strings,a2.refI,SEEK_SET);
		read(strings,&name,sizeof(char)*(a2.refF-a2.refI+1));
		name[a2.refF-a2.refI+1] = '\0';
		printf("CODE %d: NAME: %s\n",k,name);
	}	
}

void cacheSaving(){
	for (int k = 0 ; k < cachedArticles ; k++) {
		lseek(artigos,cache[k].code*sizeof(struct article)+3*sizeof(int),SEEK_SET);
		write(artigos,&cache[k].accesses,sizeof(int));
		lseek(stocks,cache[k].code*sizeof(int),SEEK_SET);
		write(stocks,&cache[k].stock,sizeof(int));
	}
}

void fileCompressor(){
	clock_t comp1 = clock();	
	int newStrings = open("newStrings", O_APPEND | O_CREAT | O_RDWR , 0644);
	cacheSaving();
	int refI,refF;
	lseek(artigos,0,SEEK_SET);
	struct article a;
	for (int k = 0 ; read(artigos,&a,sizeof(struct article)) ; k++){
		char name[2+a.refF-a.refI];
		memset(name,0,2+a.refF-a.refI);
		lseek(strings,a.refI,SEEK_SET);
		read(strings,&name,sizeof(char)*(a.refF-a.refI+1));
		name[a.refF-a.refI+2] = '\0';
		refI = (int) lseek(newStrings,0,SEEK_END);
		write(newStrings,&name,sizeof(char)*(strlen(name)));
		refF = (int) lseek(newStrings,0,SEEK_CUR)-1;
		lseek(artigos,k*sizeof(struct article),SEEK_SET);
		write(artigos,&refI,sizeof(int));
		write(artigos,&refF,sizeof(int));
		memset(name,0,a.refF-a.refI+1);
		lseek(artigos,2*sizeof(int),SEEK_CUR);
	}
	lseek(strings,0,SEEK_END);
	lseek(newStrings,0,SEEK_END);
	printf("Big file %d Compressed file %d\n",(int) lseek(strings,0,SEEK_END),(int) lseek(newStrings,0,SEEK_END));
	close(strings);
	close(newStrings);
	unlink("STRINGS");
	rename("newStrings","STRINGS");
	strings = open("STRINGS", O_CREAT | O_RDWR | O_APPEND , 0644);
	waste = 0;
	lseek(strings,0,SEEK_END);
	stringsSize = (int) lseek(strings,0,SEEK_CUR);
	clock_t comp2 = clock();	
	clock_t comp3 = comp2-comp1;	
	printf("Compression time: %ld (ms)\n", (comp3*1000)/CLOCKS_PER_SEC);

}

int compare (const void * a, const void * b){
	const cached *a1 = (const cached *)a;
	const cached *b1 = (const cached *)b;
	return (a1->accesses - b1->accesses);
}

int main(int argc, char const *argv[]){
	clock_t CPU_time_1 = clock();
	init(&userList);
	mkfifo("pipe",0666);
	int pipe = open("pipe",O_RDONLY);
	
    signal(SIGINT, aggregator);	

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
				articleToI.refI = (int) lseek(strings,0,SEEK_END);
				write(strings,&q.name,sizeof(char)*strlen(q.name));
				articleToI.refF = (int) lseek(strings,0,SEEK_CUR)-1;
				stringsSize += (articleToI.refF-articleToI.refI+1);
				lseek(artigos,0,SEEK_END);
				write(artigos,&articleToI,sizeof(articleToI));

				lseek(stocks,nextCode*sizeof(int),SEEK_SET);
				write(stocks,&articleToI.accesses,sizeof(int));
				char rep[20];
				memset(rep,0,20);
				sprintf(rep,"%d",nextCode);
				write(getPipe(userList,q.pid),rep,sizeof(rep));
				nextCode++;
			break;
			case 2: // Name changing
				lseek(artigos,q.code*sizeof(struct article),SEEK_SET);
				int repI, repF;
				read(artigos,&repI,sizeof(int));
				read(artigos,&repF,sizeof(int));
				lseek(artigos,-2*sizeof(int),SEEK_CUR);
				waste += (repF-repI+1);
				int stringI = (int) lseek(strings,0,SEEK_END);
				write(artigos,&stringI,sizeof(int));
				write(strings,&q.name,sizeof(char)*(strlen(q.name)));
				int stringF = (int) lseek(strings,0,SEEK_END)-1;
				write(artigos,&stringF,sizeof(int));
				stringsSize += (stringF-stringI+1);
				printf("stringsSize: %d and waste %d\n",stringsSize,waste);
				printf("waste percentage: %f\n", (waste*100.0/stringsSize));
				if ((waste*100.0/stringsSize) >= 20.0) fileCompressor();
			break;
			case 3: // Price changing
				lseek(artigos,q.code*sizeof(struct article)+2*sizeof(int),SEEK_SET);
				write(artigos,&q.value,sizeof(int));
				int flag3 = 0;

				for(int it3 = 0 ; it3 < cachedArticles && !flag3 ; it3++) {
					if(cache[it3].code == q.code) {
						flag3 = 1;
						cache[it3].price = q.value;
						cache[it3].accesses++;
						qsort (cache, cachedArticles, sizeof(cached), compare);											
					}
				}

				if(flag3) break; // Article was found in cache 
				
				int fileLimit3 = (int) lseek(stocks,0,SEEK_END); 
				int stockLoc3 = (int) lseek(stocks,q.code*sizeof(int),SEEK_SET);
				if (stockLoc3 > fileLimit3) flag3 = 1; // If code the User is looking for is Out of File				
				
				int stk3;
				read(stocks,&stk3,sizeof(int));
				
				if(!flag3) {
					lseek(artigos,q.code*sizeof(struct article)+3*sizeof(int),SEEK_SET);					
					int acc3;
					read(artigos,&acc3,sizeof(int));
					acc3++;
					lseek(artigos,-sizeof(int),SEEK_CUR);
					write(artigos,&acc3,sizeof(int));
					cached c;
					c.code = q.code;
					c.price = q.value;
					c.stock = stk3;
					c.accesses = acc3;
					if (cachedArticles==maxCache) { // Updating cache
						printf("acc3: %d cache[0].accesses: %d\n", acc3, cache[0].accesses);
						if (acc3 >= cache[0].accesses) {
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
					printf("Added item with code %d\n",c.code);
					qsort (cache, cachedArticles, sizeof(cached), compare);
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

				int fileLimit4 = (int) lseek(stocks,0,SEEK_END); 
				int stockLoc4 = (int) lseek(stocks,q.code*sizeof(int),SEEK_SET);
				if (stockLoc4 >= fileLimit4) flag4 = 1; // If code the User is looking for is Out of File

				if(!flag4){ // Check if the item is in file
					read(stocks,&s.stock,sizeof(int));
					lseek(artigos,q.code*sizeof(struct article)+2*sizeof(int),SEEK_SET);
					read(artigos,&s.price,sizeof(int));
					int accesses4;
					read(artigos,&accesses4, sizeof(int));
					accesses4++;

					lseek(artigos,-sizeof(int),SEEK_CUR);
					write(artigos,&accesses4,sizeof(int));
			
					cached c;
					c.code = q.code;
					c.price = s.price;
					c.stock = s.stock;
					c.accesses = accesses4;					
			
					if (cachedArticles==maxCache) { // Updating cache
						if (accesses4 >= cache[0].accesses) {
							cached repl = cache[0];
							cache[0] = c;
   							lseek(artigos,repl.code*sizeof(struct article)+3*sizeof(int),SEEK_SET);
   							write(artigos,&repl.accesses,sizeof(int));
   							lseek(stocks,repl.code*sizeof(int),SEEK_SET);
   							write(stocks,&repl.stock,sizeof(int));
						}
					}	
					else cache[cachedArticles++] = c;						
   					qsort (cache, cachedArticles, sizeof(cached), compare);
					write(getPipe(userList,q.pid),&s,sizeof(stockAndPrice));					
				}
				else{ // Item doesn't exist
					s.stock = -1;
					s.price = -1;
					write(getPipe(userList,q.pid),&s,sizeof(stockAndPrice));
				}
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
						qsort (cache, cachedArticles, sizeof(cached), compare);					
						lseek(vendas,0,SEEK_END);
						write(vendas,&s5,sizeof(sale));
						break;
					}
				}

				if (flag5) break;
				int fileLimit5 = (int) lseek(stocks,0,SEEK_END); 
				int stockLoc5 = (int) lseek(stocks,q.code*sizeof(int),SEEK_SET);
				if (stockLoc5 >= fileLimit5) flag5 = 1;

				if(!flag5){ // Check if the item is in the file
					read(stocks,&stock,sizeof(int));
					stock -= (-1*q.value);
					lseek(stocks,-sizeof(int),SEEK_CUR);
					write(stocks,&stock,sizeof(int));

					lseek(artigos,q.code*sizeof(struct article)+2*sizeof(int),SEEK_SET);
					read(artigos,&price,sizeof(int));

					s5.paidAmount = price*(-1*q.value);
				
					lseek(vendas,0,SEEK_END);
					write(vendas,&s5,sizeof(sale));

					int acc5;
					read(artigos,&acc5, sizeof(int));
					acc5++;

					lseek(artigos,-sizeof(int),SEEK_CUR);
					write(artigos,&acc5,sizeof(int));
					
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

   							lseek(artigos,repl.code*sizeof(struct article)+3*sizeof(int),SEEK_SET);
   							write(artigos,&repl.accesses,sizeof(int));

   							//printf("Article with code %d and %d accesses was replaced by article %d and %d accesses\n", repl.code, repl.accesses, c.code, c.accesses);
   							lseek(stocks,repl.code*sizeof(int),SEEK_SET);
   							//printf("Replaced written to %d\n",(int) ftell(stocks));  							
   							write(stocks,&repl.stock,sizeof(int));
   							//printf("CASE5: Wrote %d to stocks\n",repl.stock);
						}
					}	
					else cache[cachedArticles++] = c; 
					qsort (cache, cachedArticles, sizeof(cached), compare);					
				}
				else{ // Item doesn't exist
					s.stock = -1;
					s.price = -1;
					write(getPipe(userList,q.pid),&s,sizeof(stockAndPrice));
				}	

				//puts(" ");

			break;
			case 6: // User disconnecting
				kill(q.pid, SIGUSR1);
				close(getPipe(userList,q.pid));
				removeN(userList,q.pid);
			break;
		}
		//sleep(1);
	}

	cacheSaving();

	
	articleReader();
	
	/*
	struct sale sv;
	lseek(vendas,0,SEEK_SET);
	while(read(vendas,&sv,sizeof(sale))>0) printf("Code: %d Amount %d Paid Amount %d\n",sv.code,sv.quantity,sv.paidAmount);
	*/

	/*
	char buf[1024];

	lseek(strings,0,SEEK_SET);
	int retur = read(strings,&buf,1024);
	printf("buf: %sF Read %d\n",buf,retur);
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