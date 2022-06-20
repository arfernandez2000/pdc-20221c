CFLAGS = -Wall -Wextra -pedantic -pedantic-errors \
	-fsanitize=address -g -std=c11 -D_POSIX_C_SOURCE=200112L -lm $(MYCFLAGS)


all: client.o
	$(CC) client.c $(CFLAGS) -o client

.PHONY: clean

clean:
	rm -rf *.o;