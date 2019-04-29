#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h> 
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include "structures.h"

int childAmount = 0;
int maxCap = 20*sizeof(sale);
int vendas;

void merger (int i, char* time) {
	char fileName[5];
	sprintf(fileName,"agr%d",i);
	int fd = open(time,O_CREAT|O_RDWR,0644); // TIME
	int readFrom = open(fileName,O_RDONLY); // AGx
	int rdSize = (int) lseek(readFrom,0,SEEK_END);
	sale s;
	int it;
	for (it = 0 ; it*sizeof(sale) < rdSize ; it++){
		lseek(readFrom,it*sizeof(sale),SEEK_SET);
		read(readFrom,&s,sizeof(sale));
		if (s.code==it) {
			lseek(fd,s.code*sizeof(sale),SEEK_SET);
			sale p;
			read(fd,&p,sizeof(sale));
			if (p.code==s.code) {
				s.quantity += p.quantity;
				s.paidAmount += p.paidAmount;
			}
			lseek(fd,s.code*sizeof(sale),SEEK_SET);
			write(fd,&s,sizeof(sale));
		}
	}
	close(fd);
	close(readFrom);
	unlink(fileName);
}

void childProcess(int i){
	char fileName[5];
	sprintf(fileName,"agr%d",i);
	int fd = open(fileName,O_CREAT | O_RDWR, 0644);
	int processedArticles = 0, curItem;
	sale s;
	while(read(0,&s,sizeof(sale))){
		curItem *= sizeof(sale);
		lseek(fd,s.code*sizeof(sale),SEEK_SET);
		read(fd,&curItem,sizeof(int));
		if (s.code==curItem) {
			int totalSales,totalAmount;
			read(fd,&totalSales,sizeof(int));
			read(fd,&totalAmount,sizeof(int));			
			totalSales += s.quantity;
			totalAmount += s.paidAmount;
			lseek(fd,-2*sizeof(int),SEEK_CUR);	
			write(fd,&totalSales,sizeof(int));
			write(fd,&totalAmount,sizeof(int));
		}
		else {
			lseek(fd,s.code*sizeof(sale),SEEK_SET);
			write(fd,&s.code,sizeof(int));
			write(fd,&s.quantity,sizeof(int));
			write(fd,&s.paidAmount,sizeof(int));	
		}
		processedArticles++;
		lseek(fd,-sizeof(sale),SEEK_CUR);
		read(fd,&s,sizeof(sale));
	}
	close(fd);
	_exit(i);
}


int main(int argc, char const *argv[]){
	time_t rawtime;
	struct tm* timeinfo;
	time (&rawtime);
	timeinfo = localtime (&rawtime);
	char time[25];
	sprintf(time,"%d-%d-%dT%d:%d:%d",timeinfo->tm_year+1900,timeinfo->tm_mon+1,timeinfo->tm_mday,timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec);
	printf("time: %s\n",time);
	vendas = open("VENDAS",O_RDONLY);
	int saleLength = (int) lseek(vendas,0,SEEK_END);
	childAmount = (saleLength / maxCap) + 1;
	printf("sl: %d ca: %d maxCap %d\n",saleLength,childAmount,maxCap);
	if (childAmount > 10) {
		childAmount = 10;
		maxCap = (saleLength/(sizeof(sale)*10))+1;
		printf("maxCap: %d\n",maxCap);
	}
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
			default:
				wait(NULL);
			break;
		}
	}
	//int st;
	//for (int i = 0 ; i < childAmount ; i++)	waitpid(pids[i],&st,0);
	int agres = open(time,O_CREAT|O_RDWR,0644);
	sale filler;
	filler.quantity = 0;
	filler.paidAmount = 0;
	int artigos = open("ARTIGOS",O_RDONLY);
	int nextCode = ((int) lseek(artigos,0,SEEK_END)/sizeof(article));
	for(int i  = 0 ; i < nextCode-1 ; i++) {
		filler.code = i;
		write(agres,&filler,sizeof(sale));
	}
	for (int i = 0 ; i < childAmount ; i++) merger(i,time);
	int lim = (int) lseek(agres,0,SEEK_END);
	for (int it = 0 ; it*sizeof(sale) < lim ; it++) {
		lseek(agres,it*sizeof(sale),SEEK_SET);
		sale s;
		read(agres,&s,sizeof(sale));
		print_sale(s,100);
	}		
	close(agres);
	close(artigos);
	close(vendas);
	printf("agres: %s\n",time);
	return 0;
}