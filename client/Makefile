CFLAGS = -Wall -Wextra -pedantic -pedantic-errors \
	 -g -std=c11 -D_POSIX_C_SOURCE=200112L -lm -pthread $(MYCFLAGS)


all: client.o 
	$(CC) ../src/args.c client.c $(CFLAGS) -o client

.PHONY: clean

clean:
	rm -rf *.o;