CC=gcc
CFLAGS= -std=gnu99 -Wall
LDLIBS = -lrt
FLAGS = ${CFLAGS} ${LDLIBS} -o

all: prog
prog: 
	${CC} prog.c ${FLAGS} prog
.PHONY: clean
clean:
	rm prog	