.PHONY=clean all
CFLAGS = -fsanitize=address -fno-omit-frame-pointer -g --std=c11 -pedantic -pedantic-errors -Wall -Wextra -Wno-unused-parameter -Wno-implicit-fallthrough -D_POSIX_C_SOURCE=200112L 
LDFLAGS = -lpthread
all: server client
clean:	
	- rm -f *.o socks5d ssemd

COMMON =  ./args/args.o
SERVER_SOURCES = -lm ./selector/selector.o ./netutils/netutils.o ./buffer/buffer.o ./stm/stm_initialize.o ./stm/stm.o  ./stm/states/hello/hello.o ./parser/hello_parser.o ./stm/states/request/request.o ./parser/request_parser.o ./stm/states/copy/copy.o ./prawtos/prawtos.o ./parser/auth_parser.o ./stadistics/stadistics.o ./prawtos/auth_prawtos.o ./prawtos/cmd_prawtos.o ./parser/prawtos_parser.o ./user/user_utils.o 

CLIENT_SOURCES = -lm ./client/client.o 

args.o : ./args.h
buffer.o: ./buffer.h
selector.o: ./selector.h
netutils.o : ./netutils.h
buffer.o : 	./buffer.h
stm_initialize.o : ./stm_initialize.h
stm.o: ./stm.h
hello.o: ./hello.h
hello_parser.o: ./hello_parser.h
request.o: ./request.h
parser_request.o: ./parser_request.h
copy.o: ./copy.h
prawtos.o :./prawtos.h
auth_parser.o: auth_parser.h
stadistics.o: ./stadistics.h
auth_prawtos.o: ./auth_prawtos.h
cmd_prawtos.o: ./cmd_prawtos.h
prawtos_parser.o: ./prawtos_parser.h
user_utils.o: ./user_utils.h

server: $(COMMON) $(SERVER_SOURCES)
	$(CC) $(CFLAGS) $(LDFLAGS) -o server $(SERVER_SOURCES) $(COMMON)

client: $(COMMON) $(CLIENT_SOURCES)
	$(CC) $(CFLAGS) $(LDFLAGS) -o client $(CLIENT_SOURCES) $(COMMON)