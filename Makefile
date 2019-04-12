CC = gcc
CFLAGS = -Wall.

all: ma sv st

ma: ma.c
	gcc -Wall -Werror -g -o ma ma.c
sv: sv.c
	gcc -Wall -Werror -g -o sv sv.c
st: structures.h
	gcc -Wall -Werror -g -o structures structures.h
clean:
	rm -f sv ma pipe ARTIGOS.txt STRINGS.txt structures