#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h> 
#include <sys/types.h> 

int nextCode = 0;

int main(int argc, char const *argv[]){
	mkfifo("pipe",0644);
	int pipe = open("pipe",O_RDONLY);
	FILE* strings = fopen("STRINGS.txt","ab+");
	FILE* artigos = fopen("ARTIGOS.txt","ab+");

	char* buf = malloc(1024*sizeof(char));
	int n = 0;
	while ((n = read(pipe,buf,1024)) > 0){
		printf("received: %s",buf);
		switch(buf[0]){
			case 'i':
				printf("strlen:%d\n", (int) strlen(buf));
				char* token = strtok(buf," ");
				char** toI = malloc(3*sizeof(char*));
				int i = 0;
				while(token!=NULL){
					toI[i] = strdup(token);
					printf("tok:%s\n",toI[i]);
					i++;
					token = strtok(NULL," ");
				}
				fprintf(strings,"%d %s %s",nextCode++,toI[1],toI[2]);
				i--;
				while(i>=0) free(toI[i--]);
				free(toI);
				free(token);
			break;
		}
		//memset(buf,0,1024*sizeof(char));
	}

	free(buf);
	close(pipe);
	fclose(artigos);
	fclose(strings);
	return 0;
}