#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h> 
#include <sys/types.h> 
#include <signal.h>
#include "structures.h"

int signalPid = -1;
int mypid;
char fifo[15];
struct sigaction sa = {0};

void get_pid(int sig, siginfo_t *info, void *context){
    signalPid = info->si_pid;
    printf("signalPID: %d\n",signalPid);
}

void signalInit(){
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = get_pid;
    sigaction(SIGUSR2, &sa, NULL);
}

int main(int argc, char const *argv[]){
	int pipe = (int) open("pipe",O_WRONLY);
	mypid = getpid();

	signalInit();

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

   	int serverInput = open(fifo,O_RDWR);
   	if(serverInput!=-1) {
   		//dup2(serverInput,1);
   		//close(serverInput);
   	}

	char* buf = malloc(1024*sizeof(char));
	int itee =0;
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
					stockAndPrice s1;
					read(serverInput,&s1,sizeof(s1));
					printf("stock:\t%d\tprice:\t%d\n",s1.stock,s1.price);				
				break;
				case 2:;
					query case2;
					memset(&case2.name,0,128);
					case2.pid = mypid;
					case2.type = 1;
					case2.operation = 5;
					case2.code = atoi(info[0]);
					case2.value = atoi(info[1]);
					printf("before %d\n",itee);
					write(pipe,&case2,sizeof(case2));
					printf("Sent %d\n",itee++);			
					int res2;
					read(serverInput,&res2,sizeof(int));
					printf("stock:\t%d\n",res2);				
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
   	e.value=0;
   	memset(&e.name,0,128);
	write(pipe,&e,sizeof(e));

	free(buf);
	return 0;
}