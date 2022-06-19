#include <arpa/inet.h>
#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>
#include <errno.h>		
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "client.h"

#define CREDENTIALS_SIZE 255
#define CANT_OPTIONS 1

static bool done = false;
static char buff[6] = "%";

void transfered_bytes(int fd);

static void (*option_func[CANT_OPTIONS])(int fd) = {transfered_bytes};
//transfered_bytes
//historical_connections
//concurrent_connections
//get_users
//set_user
//remove_user
//change_password
//set_sniffer
//quit


static void sigterm_handler(const int signal) {
    if(signal == SIGPIPE)
        printf("Server closed connection, exiting\n");
    else
        printf("Signal %d, exiting\n", signal);
    done = true;
}

int main(int argc, char* argv[]) {


    struct socks5args args;
    parse_args(argc, argv, &args);

    struct in_addr ip_addr;
    struct in6_addr ip_addr6;
    int file_descriptor;

    //Hay que conectarse con conexion ipv4. Para eso,
    //primero hay que pasar la informacion de texto
    //a binario con la funcion inet_pton()
    if(inet_pton(AF_INET, args.mng_addr, &ip_addr)) {
        file_descriptor = connect_by_ipv4(ip_addr, htons(args.mng_port));
    } else if (inet_pton(AF_INET6, args.mng_addr, &ip_addr6)) {
        file_descriptor = connect_by_ipv6(ip_addr6, htons(args.mng_port));
    } else {
        perror("IP address is invalid");
        exit(1);
    }

    if(file_descriptor == -1) {
        perror("Connection error");
        exit(1);
	}

    // signal(SIGTERM, sigterm_handler);
    // signal(SIGINT, sigterm_handler);
    // signal(SIGPIPE, sigterm_handler);

    clean_buffer();

    printf("File Descriptor: %d\n", file_descriptor);

    first_message(file_descriptor, &args);

    options(file_descriptor);

    close(file_descriptor);
    return 0;
}

static int connect_by_ipv4(struct in_addr ip, in_port_t port) {
    int sock;
    struct sockaddr_in socket_addr;

    fprintf(stdout, "connect_ipv4");
    
    sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
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
       fprintf(stdout, "connect");
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
    
    sock = socket(PF_INET6, SOCK_STREAM, IPPROTO_TCP);
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

static void clean_buffer() {
    snprintf(buff+1, sizeof(buff)-2, "%d", CREDENTIALS_SIZE);
    int buff_len = strlen(buff);
    buff[buff_len] = 's';
    buff[buff_len+1] = '\0';
}

void first_message(int fd, socks5args *args) {
    
    char name_buffer[CREDENTIALS_SIZE];
    char password_buffer[CREDENTIALS_SIZE];
    
    bool logged_in = false;
    while (!logged_in) {
        // if (args->users->name == 0 || args->users->pass == 0) {
        //     printf("Username: ");
        //     scanf("%s", name_buffer);
        //     printf("Password: ");
        //     scanf("%s", password_buffer);
        // }
        // else {
        //     strcpy(name_buffer, args->users->name);
        //     strcpy(name_buffer, args->users->pass);
        // }
        
        // int name_len = strlen(name_buffer);
        // int password_len = strlen(password_buffer);

        // uint8_t *message = NULL;
        // message = realloc(message, 3 + name_len + password_len);
        // message[0] = 0x01;
        // message[1] = name_len;
        // strcpy(message+2, name_buffer);
        // message[2+name_len] = password_len;
        // strcpy(message+name_len+3, password_buffer);

        // send(fd, message, strlen(message), MSG_NOSIGNAL);
        // uint8_t answer[2];
        // recv(fd, answer, 2, 0);
        
        // // El primer byte de la respuesta se ignora
        // switch(answer[1]) {
        //     case 0x00:
        //         printf("Successful connection\n");
        //         logged_in = true;
        //         break;
        //     case 0x01:
        //         printf("Server failure\n");
        //         break;
        //     case 0x02:
        //         printf("Version not supported\n");
        //         break;
        //     case 0x03:
        //         printf("Incorrect username or password\n");
        //         break;
        // }
        logged_in = true;
    }
}


static void print_options() {
    system("clear");
    printf("GET: \n   1: Total bytes transfered \n   2: Historical connections\n   3: Concurrent connections\n   4: List all users\n\nSET: \n   5: Add user\n   6: Remove user\n   7: Change user password\n   8: Enable/disable password sniffer\n\n9 - Quit\n\n");
    
}

void options(int fd){
    char selected[CREDENTIALS_SIZE];
    int select;
    while(!done){
        print_options();
        printf("Choose an option:\n");
        scanf(buff, selected);

        select = atoi(selected) -1;
        if(select >=0 && select < CANT_OPTIONS && !done){
            option_func[select](fd);
        }
    }
}

void transfered_bytes(int fd){
    uint8_t request[2];

    request[0] = 0x00;
    request[1] = 0x01;
    send(fd, request, 2, 0);
}