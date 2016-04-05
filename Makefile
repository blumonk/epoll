all: main
	./a.out
main: epoll.c util.c worker.c parse.c
	gcc -std=gnu99 -Wall -Werror epoll.c util.c worker.c parse.c

