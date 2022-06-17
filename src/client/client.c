#include <arpa/inet.h>
#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>		
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int connect_by_ipv4(struct in_addr ip, in_port_t port);
static int connect_by_ipv6(struct in6_addr ip, in_port_t port);

int main(int argc, char* argv[]){
    

    char* ip = "127.0.0.1";
    uint16_t port = 8080;

    struct in_addr ip_addr;
    struct in6_addr ip_addr6;
    int file_descriptor;

    //Hay que conectarse con conexion ipv4. Para eso,
    //primero hay que pasar la informacion de texto
    //a binario con la funcion inet_pton()
    if(inet_pton(AF_INET, ip, &ip_addr)){
        file_descriptor = connect_by_ipv4(ip_addr, htons(port));
    }else if(inet_pton(AF_INET6, ip, &ip_addr6)){
        file_descriptor = connect_by_ipv6(ip_addr6, htons(port));
    }else{
        perror("IP address is invalid");
        exit(1);
    }

    if(file_descriptor == -1){
        perror("Connection error");
        exit(1);
	}

    printf("File Descriptor: %d\n", file_descriptor);
    
    close(file_descriptor);
    return 0;
}

static int connect_by_ipv4(struct in_addr ip, in_port_t port) {
    int sock;
    struct sockaddr_in socket_addr;
    
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == -1) {
        printf("Error in socket creation \n");
        return -1;
    }
    
    memset(&socket_addr, '\0', sizeof(socket_addr));
    
    socket_addr.sin_family = AF_INET;
    socket_addr.sin_port = port;
    socket_addr.sin_addr = ip;

    int answer;
    
    do {
       answer = connect(sock, (struct sockaddr*) &socket_addr, sizeof(socket_addr));
       printf("ANSWER: %d\n", answer);
       printf("errno: %d\n", errno);
       
    } while(answer == -1 && errno != EINTR);

    // En el caso de que se pudo crear la conexion pero se cierra por razones ajenas
    if(answer == -1){
        close(sock);
        return -1;
    }
    return sock;
}

static int connect_by_ipv6(struct in6_addr ip, in_port_t port) {
    int sock;
    struct sockaddr_in6 socket_addr;
    
    sock = socket(AF_INET6, SOCK_STREAM, IPPROTO_SCTP);
    if (sock == -1) {
        printf("Error in socket creation \n");
        return -1;
    }
    
    memset(&socket_addr, '\0', sizeof(socket_addr));
    
    socket_addr.sin6_family = AF_INET6;
    socket_addr.sin6_port = port;
    socket_addr.sin6_addr = ip;

    int answer;
    
    do {
       answer = connect(sock, (struct sockaddr*) &socket_addr, sizeof(socket_addr));
    } while(answer == -1 && errno != EINTR);

    // En el caso de que se pudo crear la conexion pero se cierra por razones ajenas
    if(answer == -1){
        close(sock);
        return -1;
    }
    return sock;
}