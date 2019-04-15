CC = gcc
CFLAGS = -Wall.

all: ma sv cv

ma: ma.c
	gcc -Wall -Werror -g -o ma ma.c structures.c
sv: sv.c
	gcc -Wall -Werror -g -o sv sv.c structures.c
cv: cv.c
	gcc -Wall -Werror -g -o cv cv.c structures.c
clean:
	rm -f sv ma mainpipe ARTIGOS STOCKS STRINGS VENDAS structures st pipe*