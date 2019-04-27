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

void get_pid(int sig, siginfo_t *info, void *context){
    signalPid = info->si_pid;
    printf("signalPID: %d\n",signalPid);
}

int main(int argc, char const *argv[]){
	int pipe = (int) open("pipe",O_WRONLY);
	mypid = getpid();
	struct sigaction sa = {0};
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
   	sprintf(fifo,"pipe%d",getpid());
	strcpy(q.name,fifo);
	printf("fifo: %s\n",fifo);
	write(pipe,&q,sizeof(q));
   	mkfifo(fifo,0644);
   	int serverInput = open(fifo,O_RDONLY);
   	if(serverInput!=-1) {
   		printf("serverInput:%d\n",serverInput);
   		//dup2(serverInput,1);
   		//close(serverInput);
   	}
   	else printf("serverInput:%d\n",serverInput);

	char* buf = malloc(1024*sizeof(char));
	while (getLine(0,buf,1024)) {
		printf("buf: %s\n",buf);
		if (space_counter(buf)==2) {
			char *token = strtok(buf," ");
			char** info = malloc(3*sizeof(char*));
			int it;

			for(it=0;token!=NULL;token = strtok(NULL," "),it++) info[it] = strdup(token);

			switch(info[0][0]){
				case 'i':;
					query caseI;
					memset(&caseI.name,0,128);
					caseI.pid = mypid;
					caseI.type = 0;
					caseI.operation = 1;
					caseI.code = -1;
					strcpy(caseI.name,info[1]);
					caseI.value = atoi(info[2]);
					write(pipe,&caseI,sizeof(caseI));
					char* res = malloc(1024*sizeof(char));
					read(serverInput,res,1024);
					printf("Code: %s\n",res);
					fflush(stdout);
				break;
				case 'n':;
					query caseN;
					memset(&caseN.name,0,128);
					caseN.pid = mypid;
					caseN.type = 0;
					caseN.operation = 2;
					caseN.code = atoi(info[1]);
					strcpy(caseN.name,info[2]);
					if (caseN.name[strlen(caseN.name)-1]=='\n') caseN.name[strlen(caseN.name)-1] = '\0';
					caseN.value = 0;
					write(pipe,&caseN,sizeof(caseN));
				break;
				case 'p':;
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
		}
	}
	close(serverInput);
	free(buf);
	
   	query q2;
   	q2.pid = mypid;
   	q2.type = 0; //Article Maintenance
   	q2.operation = 6; // Disconnecting
   	q2.code = -1;
   	q2.value = 0;
   	memset(&q2.name,0,128);
	write(pipe,&q2,sizeof(q2));
	close(pipe);
	printf("Beginning the wait..\n");
	pause();
	printf("Received signal\n");
	return 0;
}