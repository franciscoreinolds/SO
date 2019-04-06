CC = gcc
CFLAGS = -Wall.

all: ma sv

ma: ma.c
	gcc -Wall -Werror -g -o ma ma.c
sv: sv.c
	gcc -Wall -Werror -g -o sv sv.c

clean:
	rm -f sv ma pipe ARTIGOS.txt STRINGS.txt