#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h> 
#include <sys/types.h> 
#include <sys/wait.h>
#include <signal.h>
#include "structures.h"

int signalPid = -1;
int mypid;
char fifo[15];
int sentQueries = 0;
struct sigaction sa = {0};
struct sigaction toRead = {0};

int main(int argc, char const *argv[]){
	int pipe = (int) open("pipe",O_WRONLY);
	mypid = getpid();

   	query q;
   	q.pid = mypid;
   	q.type = 1; //Client
   	q.operation = 0; // Syncing
   	q.code = -1;
   	q.value=0;
   	memset(&q.name,0,128);
   	sprintf(fifo,"pipe%d",getpid());
	strcpy(q.name,fifo);
   	mkfifo(fifo,0644);
	write(pipe,&q,sizeof(q));

   	int rec = fork();

   	if (rec==-1) perror("Fork failed\n");

   	if (rec) { // Father
		char* buf = malloc(1024*sizeof(char));
		while (getLine(0,buf,1024)) {
			int n;
			if (strchr(buf,' ')) n = 2;	
			else n = 1;
			if (space_counter(buf)<3) {
				char** info = malloc(n*sizeof(char*));
				char *token = strtok(buf," ");
				int it;
				for(it=0;token!=NULL;token = strtok(NULL," "),it++) info[it] = strdup(token);
				switch(n){
					case 1:;
						query case1;
						memset(&case1.name,0,128);
						case1.pid = mypid;
						case1.type = 1;
						case1.operation = 4;
						case1.code = atoi(info[0]);
						case1.value = -1;
						write(pipe,&case1,sizeof(case1));
					break;
					case 2:;
						query case2;
						memset(&case2.name,0,128);
						case2.pid = mypid;
						case2.type = 1;
						case2.operation = 5;
						case2.code = atoi(info[0]);
						case2.value = atoi(info[1]);
						write(pipe,&case2,sizeof(case2));
					break;
				}
				for(it=0;it<n;it++) free(info[it]);
				free(info);
			}
		}

	   	query e;
	   	e.pid = mypid;
	   	e.type = 1; //Client
	   	e.operation = 6; // Syncing
	   	e.code = -1;
	   	e.value = 0;
	   	memset(&e.name,0,128);
		write(pipe,&e,sizeof(e));

		free(buf);	
		waitpid(rec,NULL,0);
   	}

   	else { // Child
   		int serverInput = open(fifo,O_RDONLY);   		
		reply r;
		while(read(serverInput,&r,sizeof(reply))) {
			switch(r.code){
				case 0:; //Stock checking
				char stock[16];
				snprintf(stock,sizeof(stock),"%d",r.amount);
				write(1,"Stock: ",strlen("Stock: "));
				write(1,stock,strlen(stock));

				char price[16];
				snprintf(price,sizeof(price),"%d",r.price);
				write(1," Price: ",strlen(" Price: "));
				write(1,price,strlen(price));
				write(1,"\n",1);
				break;
				
				case 1: //Stock movement
				snprintf(stock, sizeof(stock), "%d", r.amount);
				write(1,"Stock: ",strlen("Stock: "));
				write(1,stock,strlen(stock));
				write(1,"\n",1);			
				break;
				
				default:
				write(1,"Operation failed\n",strlen("Operation failed\n"));
				break;		
			}
		}
		close(serverInput);
		_exit(0);
   	}
   	wait(NULL);
	return 0;
}