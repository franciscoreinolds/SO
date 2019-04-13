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

int nextCode = 0;
int nextRef = 0;
int clientN = 0;
NODE* userList;
query q;

int main(int argc, char const *argv[]){

   	clock_t CPU_time_1 = clock();
	init(&userList);
	mkfifo("pipe",0666);
	int pipe = open("pipe",O_RDONLY);
	FILE* strings = fopen("STRINGS","ab+");
	FILE* artigos = fopen("ARTIGOS","wb+");

	int n;
	while((n=read(pipe,&q,sizeof(q)))>0){
		switch(q.operation){
			case 0: // Connection establishment
				printf("The user that wants to connect is of type %d and has a pid of %d\n",q.type,q.pid);			
				user element;
				element.pid = q.pid;
				strcpy(element.namedPipe,q.name);
				element.type = q.type;
				element.fd = open(q.name,O_RDWR);
				printf("Opening pipe %s numbered %d\n",q.name,element.fd);
				userList = add(userList,element);
			 	print_list(userList);				
			break;
			case 1: // Inserting a new article
				printf("The article to insert is called %s and costs %d\n",q.name,q.value);
				puts(" ");
				article articleToI;
				articleToI.price = q.value;
				articleToI.accesses=0;	
				articleToI.ref = ftell(strings);
				
				fseek(strings,ftell(strings),SEEK_CUR);
				printf("ref:%d\n",articleToI.ref);
				fwrite(q.name,1,strlen(q.name),strings);
				printf("after:%d\n",(int) ftell(strings));
				fwrite(&articleToI,sizeof(articleToI),1,artigos);
				char rep[20];
				sprintf(rep,"%d",articleToI.ref);
				write(getPipe(userList,q.pid),rep,sizeof(rep));
				//write(getPipe(userList,q.pid),"Server",sizeof("Server"));
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
			break;
			case 6: // User disconnecting
				printf("Case 6\n");
				sleep(2);
				kill(q.pid, SIGUSR1);
				close(getPipe(userList,q.pid));
				removeN(userList,q.pid);
				puts("");
			break;
		}
	}

	puts("------------------------------------");
	fseek(artigos,0,SEEK_SET);
	struct article *a2 = malloc(sizeof(struct article));
	while(fread(a2,sizeof(struct article),1,artigos)) printf("a1.ref=%d, a1.price=%d, a1.accesses=%d\n",a2->ref,a2->price,a2->accesses);
	close(pipe);
	puts(" ");
	fclose(artigos);
	fclose(strings);
	unlink("pipe");
	print_list(userList);
 	userList = free_list(userList);	
    
    clock_t CPU_time_2 = clock();
    clock_t time_3 = (double) CPU_time_2 - CPU_time_1;
    printf("CPU end time is : %ld (s)\n", time_3/CLOCKS_PER_SEC);

	return 0;
}