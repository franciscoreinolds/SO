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
		printf("refF %d refI %d\n", a.refF, a.refI);
		char name[2+a.refF-a.refI];
		memset(name,0,2+a.refF-a.refI);
		lseek(strings,a.refI,SEEK_SET);
		read(strings,&name,sizeof(char)*(a.refF-a.refI+1));
		//name[a.refF-a.refI+2] = '\0';
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
	stringsSize = (int) lseek(strings,0,SEEK_END)-1;
	clock_t comp2 = clock();	
	clock_t comp3 = comp2-comp1;	
	printf("Compression time: %ld (ms)\n", (comp3*1000)/CLOCKS_PER_SEC);

}

void articleReader(){
	int lim = (int) lseek(artigos,0,SEEK_END);
	lseek(artigos,0,SEEK_SET);
	struct article a2;
	printf("lim: %d article %d\n",lim, (int) sizeof(article));
	for (int k = 0;read(artigos,&a2,sizeof(article));k++){
		int stock;
		lseek(stocks,k*sizeof(int),SEEK_SET);
		read(stocks,&stock,sizeof(int));
		printf("CODE %d: a1.refI=%d\ta1.refF=%d\ta1.price=%d\ta1.accesses=%d\tand final stock %d\n",k,a2.refI,a2.refF,a2.price,a2.accesses,stock);
		char name[1+a2.refF-a2.refI];
		lseek(strings,a2.refI,SEEK_SET);
		read(strings,&name,sizeof(char)*(a2.refF-a2.refI+1));
		name[a2.refF-a2.refI+1] = '\0';
		printf("CODE %d: NAME: %s\n",k,name);
		puts(" ");
	}	
}

void variableSetup(){
	article a;
	stringsSize = (int) lseek(strings,0,SEEK_END);
	printf("stringsSize: %d\n",stringsSize);	
	int used = 0;
	for (int k = 0;read(artigos,&a,sizeof(article));k++) waste += a.refF-a.refI+1;
	printf("Waste: %d Total %d\n",used,stringsSize);
	nextCode = (lseek(artigos,0,SEEK_END)/sizeof(article));
	printf("NextCode: %d\n",nextCode);
}

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
   	mkfifo(fifo,0644);

	write(pipe,&q,sizeof(q));
   	int serverInput = open(fifo,O_RDONLY);

   	pause();

	artigos = open("ARTIGOS", O_RDWR);
	strings = open("STRINGS", O_APPEND | O_RDWR);
	stocks = open("STOCKS", O_RDWR);
	if(lseek(artigos,0,SEEK_END)) variableSetup();

	sleep(2);

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
					printf("refI: %d refF: %d\n",articleToI.refI,articleToI.refF);

					lseek(artigos,nextCode*sizeof(article),SEEK_SET);
					write(artigos,&articleToI,sizeof(articleToI));

					lseek(stocks,nextCode*sizeof(int),SEEK_SET);
					write(stocks,&articleToI.accesses,sizeof(int));

					printf("Code: %d\n",nextCode);
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
					printf("Wrote from %d to %d\n", stringI,stringF);
					lseek(artigos,code*sizeof(article),SEEK_SET);					
					write(artigos,&stringI,sizeof(int));
					write(artigos,&stringF,sizeof(int));
					stringsSize += (stringF-stringI+1);

					printf("stringsSize: %d and waste %d\n",stringsSize,waste);
					printf("waste percentage: %f\n", (waste*100.0/stringsSize));
					if ((waste*100.0/stringsSize) >= 20.0) fileCompressor();
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

	query q2;
   	q2.pid = mypid;
   	q2.type = 0; //Article Maintenance
   	q2.operation = 4; // Disconnecting
   	q2.code = -1;
   	q2.value = 0;
   	memset(&q2.name,0,128);
	write(pipe,&q2,sizeof(q2));
	close(pipe);
	
	articleReader();

	close(artigos);
	close(stocks);
	close(strings);

	close(serverInput);
	free(buf);

	return 0;
}