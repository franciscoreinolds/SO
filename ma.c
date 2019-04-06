#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h> 
#include <sys/types.h> 

int main(int argc, char const *argv[]){
	int pipe = open("pipe",O_WRONLY);
	char* buf = malloc(1024*sizeof(char));
	int n = 0;
	while ((n = read(0,buf,1024))>0 && (strncmp("q",buf,1))!=0){
		write(pipe,buf,1024);
		memset(buf,0,1024);
	}
	close(pipe);
	return 0;
}