#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h> 
#include <sys/types.h> 

int nextCode = 0;
int nextRef = 0;
int clientN = 0;

int main(int argc, char const *argv[]){
	mkfifo("pipe",0644);
	int pipe = open("pipe",O_RDONLY);
	FILE* strings = fopen("STRINGS.txt","a+");
	FILE* artigos = fopen("ARTIGOS.txt","a+");
	FILE* tubo = fdopen(pipe,"r");

	char* buf = malloc(1024*sizeof(char));

    /*
    int i = 0;
    char line[256];
    while (fgets(line, sizeof(line), strings)) {
        i++;
        if(i<4) printf("Linha %d: %s", i, line);   
    }
	*/

	while (fgets(buf, 1024, tubo)) {
		printf("received: %s\n",buf);
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
				
				fprintf(strings,"%d %s %s",nextCode,toI[1],toI[2]);
				fprintf(artigos,"%d %d\n",nextCode,nextRef);
				printf("Código inserido: %d\n",nextCode);
				i--;
				nextCode++;
				nextRef++;

				while(i>=0) free(toI[i--]);
				free(toI);
				free(token);
				rewind(artigos);
				rewind(strings);
			break;
			case 'n':;
				int r;
				r = buf[2] - '0';
				printf("O n: %d\n",r);
				int codigo, referencia = -1;
				while (fscanf(artigos,"%d %d\n", &codigo, &referencia)!=EOF) if (codigo==r) break;
				if (referencia != -1) {
					char* toChange = malloc(1024*sizeof(char));
					int code, ref, count = 0;
					rewind(strings);
					//while ((n = fscanf(strings,"%d %s %d\n", &code, toChange, &ref))==3) printf("n %d apanhado: %d %s %d\n",n,code,toChange,ref);
					while(fscanf(strings,"%d %s %d\n", &code, toChange, &ref)==3) {
							printf("count: %d referencia: %d\n", count, referencia);
							printf("apanhado: %d %s %d\n",code,toChange,ref);
						    if (count == referencia) {
						    	fprintf(strings,"%d %s %d\n",code,"ISTO NAO E LIMONADA",ref);
						    	printf("Inserido: %d %s %d\n",code,toChange,ref);
					    }
					    else count++;
					   	memset(toChange,0,1024);
					}
					/*
					while (fgets(toChange,1024, file)!= NULL){

					}
					*/
					free(toChange);
				}
				else break;
				rewind(strings);
				rewind(artigos);
			break;
			case 'p':
			break;
		}
		memset(buf,0,1024);
	}

	/*
	while ((n = read(pipe,buf,1024)) > 0){
	}
	*/

	free(buf);
	close(pipe);
	fclose(tubo);
	fclose(artigos);
	fclose(strings);
	unlink("pipe");
	return 0;
}