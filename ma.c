#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h> 
#include <sys/types.h> 
#include <signal.h>
#include <time.h>
#include "structures.h"

int signalPid = -1;
int mypid;
int nextCode = 0;
int namedPipe;
int stringsSize = 0;
int waste = 0;
char fifo[15];

int artigos;
int stocks;
int strings;

void fileCompressor(){
	clock_t comp1 = clock();	
	int newStrings = open("newStrings", O_APPEND | O_CREAT | O_RDWR , 0644);
	int refI,refF;
	lseek(artigos,0,SEEK_SET);
	struct article a;
	for (int k = 0 ; read(artigos,&a,sizeof(struct article)) ; k++){
		char name[2+a.refF-a.refI];
		memset(name,0,2+a.refF-a.refI);
		lseek(strings,a.refI,SEEK_SET);
		read(strings,&name,sizeof(char)*(a.refF-a.refI+1));
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
	close(strings);
	close(newStrings);
	unlink("STRINGS");
	rename("newStrings","STRINGS");
	strings = open("STRINGS", O_CREAT | O_RDWR | O_APPEND , 0644);
	waste = 0;
	lseek(strings,0,SEEK_END);
	stringsSize = (int) lseek(strings,0,SEEK_END)-1;
	clock_t comp2 = clock();	
	clock_t comp3 = comp2-comp1;	
	printf("Compression time: %ld (ms)\n", (comp3*1000)/CLOCKS_PER_SEC);

	query q;
	memset(&q.name,0,128);
	q.pid = mypid;
	q.type = 0;
	q.operation = 1;
	q.code = 0;
	q.value = 0;
	write(namedPipe,&q,sizeof(q));	
}

void articleReader(){
	lseek(artigos,0,SEEK_SET);
	struct article a2;
	for (int k = 0;read(artigos,&a2,sizeof(article));k++){
		int stock;
		lseek(stocks,k*sizeof(int),SEEK_SET);
		read(stocks,&stock,sizeof(int));
		printf("CODE %d: a1.refI=%d\ta1.refF=%d\ta1.price=%d\ta1.accesses=%d\tand final stock %d\n",k,a2.refI,a2.refF,a2.price,a2.accesses,stock);
		char name[1+a2.refF-a2.refI];
		lseek(strings,a2.refI,SEEK_SET);
		read(strings,&name,sizeof(char)*(a2.refF-a2.refI+1));
		name[a2.refF-a2.refI+1] = '\0';
		printf("CODE %d: NAME: %s\n\n",k,name);
	}	
}

void variableSetup(){
	article a;
	stringsSize = (int) lseek(strings,0,SEEK_END);
	for (int k = 0;read(artigos,&a,sizeof(article));k++) waste += a.refF-a.refI+1;
	nextCode = (lseek(artigos,0,SEEK_END)/sizeof(article));
}

void get_pid(int sig, siginfo_t *info, void *context){
    signalPid = info->si_pid;
}

int main(int argc, char const *argv[]){
	namedPipe = open("pipe",O_WRONLY);

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
   	mkfifo(fifo,0644);
   	int serverInput = open(fifo,O_RDWR);	
	write(namedPipe,&q,sizeof(q));

   	pause();

	artigos = open("ARTIGOS", O_RDWR);
	strings = open("STRINGS", O_APPEND | O_RDWR);
	stocks = open("STOCKS", O_RDWR);
	if(lseek(artigos,0,SEEK_END)) variableSetup();


	char* buf = malloc(1024*sizeof(char));
	while (getLine(0,buf,1024)) {
		if (space_counter(buf)==2) {
			char *token = strtok(buf," ");
			char** info = malloc(3*sizeof(char*));
			int it;

			for(it=0;token!=NULL;token = strtok(NULL," "),it++) info[it] = strdup(token);

			switch(info[0][0]){
				case 'i':;
					article articleToI;
					articleToI.price = atoi(info[2]);
					articleToI.accesses=0;
					articleToI.refI = (int) lseek(strings,0,SEEK_END);
					
					write(strings,info[1],sizeof(char)*strlen(info[1]));
					articleToI.refF = (int) lseek(strings,0,SEEK_CUR)-1;
					stringsSize += (articleToI.refF-articleToI.refI+1);

					lseek(artigos,nextCode*sizeof(article),SEEK_SET);
					write(artigos,&articleToI,sizeof(articleToI));

					lseek(stocks,nextCode*sizeof(int),SEEK_SET);
					write(stocks,&articleToI.accesses,sizeof(int));

					nextCode++;
				break;
				case 'n':;
					int code = atoi(info[1]);
					lseek(artigos,code*sizeof(article),SEEK_SET);
					int repI, repF;
					read(artigos,&repI,sizeof(int));
					read(artigos,&repF,sizeof(int));
					waste += (repF-repI+1);
					int stringI = (int) lseek(strings,0,SEEK_END);
					write(strings,info[2],sizeof(char)*(strlen(info[2])));
					int stringF = (int) lseek(strings,0,SEEK_END)-1;
					lseek(artigos,code*sizeof(article),SEEK_SET);					
					write(artigos,&stringI,sizeof(int));
					write(artigos,&stringF,sizeof(int));
					stringsSize += (stringF-stringI+1);

					if ((waste*100.0/stringsSize) >= 20.0) fileCompressor();					
				break;
				case 'p':;
					query caseP;
					memset(&caseP.name,0,128);
					caseP.pid = mypid;
					caseP.type = 0;
					caseP.operation = 2;
					caseP.code = atoi(info[1]);
					caseP.value = atoi(info[2]);
					write(namedPipe,&caseP,sizeof(caseP));					
				break;
			}

			for(it=0;it<3;it++) free(info[it]);
			free(info);
		
		}
		else if (buf[0] == 'a' && buf[1] == '\0') {
			query caseA;
			memset(&caseA.name,0,128);
			caseA.pid = mypid;
			caseA.type = 0;
			caseA.operation = 3;
			caseA.code = 0;
			caseA.value = 0;
			write(namedPipe,&caseA,sizeof(caseA));				
		}
	}

	query q2;
   	q2.pid = mypid;
   	q2.type = 0; //Article Maintenance
   	q2.operation = 6; // Disconnecting
   	q2.code = -1;
   	q2.value = 0;
   	memset(&q2.name,0,128);
	write(namedPipe,&q2,sizeof(q2));

	close(namedPipe);
	
	articleReader();

	close(artigos);
	close(stocks);
	close(strings);

	close(serverInput);
	free(buf);

	return 0;
}