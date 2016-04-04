main: epoll.c util.c
	gcc -std=c99 -Wall -Werror epoll.c util.c
