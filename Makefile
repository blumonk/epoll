main: epoll.c util.c
	gcc -std=gnu99 -Wall -Werror epoll.c util.c

