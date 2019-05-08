CC = gcc
CFLAGS = -Wall.

all: ag ma sv cv

ag: ag.c
	gcc -Wall -Werror -g -o ag ag.c structures.c
ma: ma.c
	gcc -Wall -Werror -g -o ma ma.c structures.c
sv: sv.c
	gcc -Wall -Werror -g -o sv sv.c structures.c
cv: cv.c
	gcc -Wall -Werror -g -o cv cv.c structures.c
clean:
	rm -f ag cv sv ma mainpipe ARTIGOS newStrings STOCKS STRINGS VENDAS structures st pipe* agr* 2019*
	rm -r AgFiles ReadableAg