HEADERS=include
SRC=src

HFILES=$(shell find $(HEADERS) -name '*.h' | sed 's/^.\///')

FILES=$(shell find $(SRC) -name '*.c' | sed 's/^.\///')
OFILES=$(patsubst %.c,./%.o,$(FILES))

CFLAGS = -Wall -Wextra -pedantic -pedantic-errors -O3 -std=c11 -D_POSIX_C_SOURCE=200112L -pthread $(MYCFLAGS)

DEBUG_FLAGS = -Wall -Wextra -pedantic -pedantic-errors \
	-fsanitize=address -g -std=c11 -D_POSIX_C_SOURCE=200112L $(MYCFLAGS)

%.o: %.c $(HFILES)
	$(CC) -c -o $@ $< $(CFLAGS)

all: server

server: $(OFILES)
	$(CC) $(OFILES) $(CFLAGS) -o server

debug: 
	$(CC) $(FILES) $(DEBUG_FLAGS) -o server

client:
	cd client; make all;

.PHONY: clean

clean: 
	rm -rf $(OFILES); cd client; make clean;
