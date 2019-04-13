CC = gcc
CFLAGS = -Wall.

all: ma sv

ma: ma.c
	gcc -Wall -Werror -g -o ma ma.c structures.c
sv: sv.c
	gcc -Wall -Werror -g -o sv sv.c structures.c
clean:
	rm -f sv ma mainpipe ARTIGOS STRINGS structures st pipe*