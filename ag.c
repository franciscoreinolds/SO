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
#include <limits.h>
#include <fts.h>
#include "structures.h"

int childAmount = 0;
int maxCap = 20*sizeof(sale);
int vendas;

void merger (char* time, char ag[PATH_MAX]) {
	int fd = open(time, O_CREAT | O_RDWR, 0644);
	char fileN[5];
	int readFrom;
	int rdSize;
	int n;
	if (ag != NULL) n = -1;
	else n = 0;
	for(int i = n; i<childAmount; i++){
		if (i == -1) {
			readFrom = open(ag, O_RDONLY);
		}
		else {
			sprintf(fileN, "agr%d", i);
			readFrom = open(fileN,O_RDONLY);
		}
		rdSize = (int) lseek(readFrom,0,SEEK_END);
		sale s;
		for(int it = 0; it*sizeof(sale) < rdSize; it++){
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
		close(readFrom);
		if (i != -1) unlink(fileN);
	}
	close(fd);	
}

char *findRecent(){
	char cwd[PATH_MAX];
	getcwd(cwd, sizeof(cwd)); // cwd determines the path to the current direcotory. Saves current directory on cwd.
	strcat(cwd, "/AgFiles/"); // appends to the directory

	FTS* file_system = NULL;
	FTSENT* child = NULL;
	FTSENT* parent = NULL;
	time_t fileT = 0;
	char fileN[20];

	// COMFOLLOW causes any symbolic link specified as a root path to be followed immediately
	// FTS_NOCHDIR turns of a optimization that would change the current directory as it accesses files;
	// NULL is in place of a compare function that would dictate the order in which the files are accessed.
	char * const path[2] = {cwd, NULL};
	file_system = fts_open(path, FTS_COMFOLLOW | FTS_NOCHDIR,NULL);
	if (file_system != NULL){
		while ((parent = fts_read(file_system)) != NULL) {
			child = fts_children(file_system,0);
			while(child != NULL) {
				if (fileT < (child->fts_statp)->st_mtime) {
					fileT = (child->fts_statp)->st_mtime;
					strcpy(fileN,child->fts_name);
				}
				child = child->fts_link;
			}
		}
		fts_close(file_system);
	}
	strcat(cwd, fileN);
	printf("%s\n", cwd);

	char *rtn = cwd;
	return rtn;
}

void childProcess(int i){
	char fileName[5];
	sprintf(fileName,"agr%d",i);
	int fd = open(fileName,O_CREAT | O_RDWR, 0644);
	int processedArticles = 0, curItem;
	sale s;
	while(read(0,&s,sizeof(sale))){
		//printf("Read\n");
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
	vendas = open("VENDASAG",O_RDONLY);
	int saleLength = (int) lseek(vendas,0,SEEK_END);
	childAmount = (saleLength / maxCap) + 1;
	if (childAmount > 8) {
		childAmount = 8;
		maxCap = (saleLength/(sizeof(sale)*childAmount))+1;
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
		}
	}
	int st;
	for (int i = 0 ; i < childAmount ; i++)	waitpid(pids[i],&st,0);
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

	//new
	char cwd[PATH_MAX];
	getcwd(cwd, sizeof(cwd)); // cwd determines the path to the current direcotory. Saves current directory on cwd.
	strcat(cwd, "/AgFiles/"); // appends to the directory
	char rcwd[PATH_MAX];
	getcwd(rcwd, sizeof(rcwd)); // cwd determines the path to the current direcotory. Saves current directory on cwd.
	strcat(rcwd, "/ReadableAg/");
	struct stat check = {0}; // stat function requires a static struct
	if (stat(cwd, &check) == -1) { // returns 0 on success and -1 on error. Checks if it exists
		mkdir(cwd, 0700);
		mkdir(rcwd,0700);
		merger(time, NULL);
	}
	else{
		merger(time, findRecent());
	}
	
	int lim = (int) lseek(agres,0,SEEK_END);
	int tmp = open("tmpf", O_CREAT | O_RDWR, 0644);
	dup2(tmp,1);
	for (int it = 0 ; it*sizeof(sale) < lim ; it++) {
		lseek(agres,it*sizeof(sale),SEEK_SET);
		sale s;
		read(agres,&s,sizeof(sale));
		print_sale(s,100);
	}

	close(tmp);
	close(agres);
	close(artigos);
	close(vendas);
	strcat(cwd,time);
	strcat(rcwd,time);
	rename("tmpf",rcwd);

	rename(time, cwd);
	remove("VENDASAG");
	return 0;
}