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
char fifo[7];

void get_pid(int sig, siginfo_t *info, void *context){
    signalPid = info->si_pid;
    printf("signalPID: %d\n",signalPid);
}

int main(int argc, char const *argv[]){
	int pipe = open("pipe",O_WRONLY);
	mypid = getpid();
	printf("PID MA: %d\n", getpid());       //display PID for kill()

   	struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = get_pid;
    sigaction(SIGUSR2, &sa, NULL);

   	query q;
   	q.pid = mypid;
   	q.type = 0; //Article Maintenance
   	q.operation = 0; // Syncing
   	q.code = -1;
   	q.value=0;
   	memset(&q.name,0,128);
   	sprintf(fifo,"%d",getpid());
	strcpy(q.name,fifo);
	write(pipe,&q,sizeof(q));
   	
   	printf("PID: %d\n",getpid());

	char* buf = malloc(1024*sizeof(char));
	
	while (fgets(buf, 1024, stdin)) {
		char *token = strtok(buf," ");
		char** info = malloc(3*sizeof(char*));
		int it;

		for(it=0;token!=NULL;token = strtok(NULL," "),it++) info[it] = strdup(token);

		switch(info[0][0]){
			case 'i':
				printf("Case i\n");
				for(it=0;it<3;it++) printf("info[%d]:%s\n",it,info[it]);
				query caseI;
				memset(&caseI.name,0,128);
				caseI.pid = mypid;
				caseI.type = 0;
				caseI.operation = 1;
				caseI.code = -1;
				strcpy(caseI.name,info[1]);
				caseI.value = atoi(info[2]);
				write(pipe,&caseI,sizeof(caseI));
			break;
			case 'n':
				printf("Case n\n");
				for(it=0;it<3;it++) printf("info[%d]:%s\n",it,info[it]);
				query caseN;
				memset(&caseN.name,0,128);
				caseN.pid = mypid;
				caseN.type = 0;
				caseN.operation = 2;
				caseN.code = atoi(info[1]);
				strcpy(caseN.name,info[2]);
				caseN.value = 0;
				write(pipe,&caseN,sizeof(caseN));
			break;
			case 'p':
				printf("Case p\n");
				for(it=0;it<3;it++) printf("info[%d]:%s\n",it,info[it]);
				query caseP;
				memset(&caseP.name,0,128);
				caseP.pid = mypid;
				caseP.type = 0;
				caseP.operation = 3;
				caseP.code = atoi(info[1]);
				caseP.value = atoi(info[2]);
				write(pipe,&caseP,sizeof(caseP));					
			break;
		}	


		for(it=0;it<3;it++) free(info[it]);

		free(info);

		/*
	   	struct query *q = malloc(sizeof(struct query));
	   	q->pid = getpid();
	   	q->type = 0; //Client
	   	q->operation = 0; //Syncing
	   	write(pipe,q,sizeof(q));
	   	*/
	}

	free(buf);
	close(pipe);
	return 0;
}