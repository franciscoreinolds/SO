#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

int nextCode = 0;
int i = 0;

char** parse(char* buf){
	char* token1 = strtok(buf,"\n");
	char* token = strtok(token1," ");
	char** toI = malloc(3*sizeof(char*));
	i=0;
	while(token!=NULL ){
		toI[i] = strdup(token);
		printf("tok:%s\n",toI[i]);
		i++;
		token = strtok(NULL," ");
	}
		i--;
	// toI[i][(strlen(toI[i])-1)] ='\0';

	free(token);

	return toI;
}


int main(int argc, char const *argv[]){
	mkfifo("pipe",0644);
	int pipe = open("pipe",O_RDONLY);
	int strings = open("STRINGS.txt",O_CREAT | O_RDWR, 0666);
	int artigos = open("ARTIGOS.txt",O_CREAT | O_RDWR, 0666);

	//char* aux = malloc(1024*sizeof(char));
	char* buf = malloc(1024*sizeof(char));
	char* buf2 = malloc(1024*sizeof(char));


	char** input = malloc(3*sizeof(char*));
	char** old = malloc(3*sizeof(char*));
	int n = 0;

	int a=0;
	while ((n = read(pipe,buf,1024)) > 0){
		printf("received: %s",buf);
		switch(buf[0]){
			case 'i':
				printf("strlen:%d\n", (int) strlen(buf));
				input=parse(buf);
				char* linha_nova = malloc(1024*sizeof(char));
				sprintf(linha_nova, "%d", nextCode++);
				strcat(linha_nova," ");
				strcat(linha_nova,input[1]);
				strcat(linha_nova," ");
				strcat(linha_nova,input[2]);
				strcat(linha_nova,"\n");
				printf("linha_nova: %s",linha_nova);
				write(strings,linha_nova,(int) strlen(linha_nova));

				while(i>=0) free(input[i--]);
				free(input);
				free(linha_nova);
				break;
			case 'n':
				printf("strlen:%d\n", (int) strlen(buf));
				input=parse(buf);
				lseek(strings, atoi(input[1]) , SEEK_SET);
				a = read(strings,buf2,1024); // help me here :)
				if(a>0){	
					old=parse(buf2);
					int d = strlen(old[1])-strlen(input[2]);
					//lseek(strings,atoi(input[1]),SEEK_SET);
					char* linha_nova2 = malloc(1024*sizeof(char));
					strcat(linha_nova2,input[1]);
					strcat(linha_nova2," ");
					strcat(linha_nova2,input[2]);
					strcat(linha_nova2," ");
					strcat(linha_nova2,old[2]);
					while(d<0){
					strcat(linha_nova2," ");
					d++;
					}
					strcat(linha_nova2,"\n");
					//lseek(strings, 0, SEEK_END);
					lseek(strings,atoi(input[1]),SEEK_SET);
					printf("linha_nova2: %s",linha_nova2);
					write(strings,linha_nova2,(int) strlen(linha_nova2));
					free(linha_nova2);
				}
				lseek(strings, 0, SEEK_END);
				printf("Sai: %s\n", strerror(errno));


			break;
		}
		//memset(buf,0,1024*sizeof(char));
	}

	free(buf);
	free(buf2);
	close(pipe);
	close(artigos);
	close(strings);
	return 0;
}
