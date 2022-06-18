#include <stdio.h>
#include <sys/socket.h> 
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "args/args.h"
#include "netutils/netutils.h"
#include "selector/selector.h"
#include "socks5/socks5.h"

#define DEFAULT_TIMEOUT 5
#define SELECTOR_COUNT 1024
#define SOCKET_BACKLOG 100

typedef struct server_handler {
    struct in_addr ipv4addr;
    struct in6_addr ipv6addr;
    in_port_t port;
    fd_handler ipv6_handler;
    fd_handler ipv4_handler;
    fd_handler adminHandler;
    int ipv4Fd;
    int ipv6Fd;
    int adminFd;
} server_handler;

static socks5args args;

static server_handler serverHandler;
static int generate_socket(struct sockaddr * addr, socklen_t addr_len);
static int generate_socket_ipv4(fd_selector selector);
static int generate_socket_ipv6(fd_selector selector);
void listen_interfaces(fd_selector selector);
static fd_selector init_selector();


int main(const int argc, char **argv) {

    static bool done = false;
    
    parse_args(argc, argv, &args);

    serverHandler.port = htons(60178);

    close(STDIN_FILENO);

    selector_fd_set_nio(STDOUT_FILENO);
    selector_fd_set_nio(STDERR_FILENO);

    selector_status ss = SELECTOR_SUCCESS;
    fd_selector selector = init_selector();

    if(selector == NULL)
        return -1;
    
    initialize_socks5(&args, selector);

    listen_interfaces(selector);

    for(;!done;) {
        ss = selector_select(selector);
        if(ss != SELECTOR_SUCCESS) {
            perror("Fallo en el while 1");
        }
    }
}

static fd_selector init_selector(){

    const struct selector_init init = {
        .signal = SIGALRM,
        .select_timeout = {
            .tv_sec = DEFAULT_TIMEOUT,
            .tv_nsec = 0,
        },
    };
    
    if(selector_init(&init)){
        perror("Error initializing selector");
        return NULL;
    }
    
    fd_selector new_selector = selector_new(SELECTOR_COUNT);
    if(new_selector == NULL){
        perror("couldn't create selector");
        return NULL;
    }
    
    return new_selector;

}


void listen_interfaces(fd_selector selector) {
    serverHandler.ipv6addr = in6addr_any;
    serverHandler.ipv4addr.s_addr = htonl(INADDR_ANY);
    generate_socket_ipv4(selector);
    generate_socket_ipv6(selector);
}

static int generate_socket_ipv4(fd_selector selector){
    memset(&serverHandler.ipv4_handler, '\0', sizeof(serverHandler.ipv4_handler));

    // LLamo a la funcioan que socks5 a ejecutar
    serverHandler.ipv4_handler.handle_read = new_connection_ipv4;

    struct sockaddr_in sockaddr4 = {
        .sin_addr = serverHandler.ipv4addr,
        .sin_family = AF_INET,
        .sin_port = serverHandler.port,
    };

    int ipv4Fd = generate_socket((struct sockaddr *)&sockaddr4, sizeof(sockaddr4));
    if(ipv4Fd == -1) {
        return -1;
    }

    if(selector_register(selector, ipv4Fd, &serverHandler.ipv4_handler, OP_READ, NULL)){
        perror("Failed file descriptor registration for ipv4");
        return -1;
    }

    serverHandler.ipv4Fd = ipv4Fd; 
    return 0;
}

static int generate_socket_ipv6(fd_selector selector){
    memset(&serverHandler.ipv6_handler, '\0', sizeof(serverHandler.ipv6_handler));

    // LLamo a la funcioan que socks5 a ejecutar
    serverHandler.ipv6_handler.handle_read = new_connection_ipv6;

    struct sockaddr_in6 sockaddr6 = {
        .sin6_addr = serverHandler.ipv6addr,
        .sin6_family = AF_INET6,
        .sin6_port = serverHandler.port,
    };

    int ipv6Fd = generate_socket((struct sockaddr *)&sockaddr6, sizeof(sockaddr6));
    if(ipv6Fd == -1) {
        return -1;
    }

    if(selector_register(selector, ipv6Fd, &serverHandler.ipv6_handler, OP_READ, NULL)){
        perror("Failed file descriptor registration for ipv6");
        return -1;
    }

    serverHandler.ipv6Fd = ipv6Fd; 
    return 0;
}

static int generate_socket(struct sockaddr * addr, socklen_t addr_len){

    const int fd = socket(addr->sa_family, SOCK_STREAM, IPPROTO_TCP);
    if(fd < 0){
        perror("Failed creating socket");
        return -1;
    }

    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &(int) {1}, sizeof(int));
    int selector_set = selector_fd_set_nio(fd);
    
    if (bind(fd, addr, addr_len) < 0) {
        perror("Failed to bind socket to sockaddr");
        return -1;
    }
    
    if (listen(fd, SOCKET_BACKLOG) < 0) {
        perror("Listen failed");
        return -1;
    }
    
    if(selector_set == -1){
        perror("Failed getting server socket flags");
        return -1;
    }

    return fd;
    
}

// static int generate_socket_ipv6(fd_selector selector){
//  
//      memset(&server_handler.ipv4_handler, '\0', sizeof(server_handler.ipv4_handler));
// 
// }