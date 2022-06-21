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

#include "../include/args.h"

#define CREDENTIALS_SIZE 255
#define CANT_OPTIONS 10

static bool done = false;
static char buff[6] = "%";

void transfered_bytes(int fd);
void connection_history(int fd);
void concurrent_connections(int fd);
void retrieve_users(int fd);
void create_user(int fd);
void create_admin(int fd);
void remove_user(int fd);
void modify_user(int fd);

void quit(int fd);

void set_sniffer(int fd);

void welcome();
void get_command(int fd, uint8_t cmd, char *operation);
static int connect_by_ipv4(struct in_addr ip, in_port_t port);
static int connect_by_ipv6(struct in6_addr ip, in_port_t port);
void first_message(int fd, socks5args *args);
static void clean_buffer();
void options(int fd);
void add_new_user(int fd, bool admin);
void user_answer_handler(int fd, uint8_t *answer);
void get_answer_handler(int fd, uint8_t *answer, char **result);
void get_other_users(int fd, char **result, uint8_t *answer);



static void (*option_func[CANT_OPTIONS])(int fd) = {transfered_bytes, connection_history, concurrent_connections, retrieve_users, create_user, create_admin, remove_user, modify_user, set_sniffer, quit};

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
        // fprintf(stdout, "ip_addr: %u\n", ip_addr.s_addr);
        // fprintf(stdout, "args.mng_port: %d\n", args.mng_port);
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
    
    welcome();

    first_message(file_descriptor, &args);
    options(file_descriptor);

    close(file_descriptor);
    return 0;
}

void welcome() {
    printf("\nWelcome to the Prawtos client!\n");
    printf("Please sign in with your credentials\n");
}

static int connect_by_ipv4(struct in_addr ip, in_port_t port) {
    int sock;
    struct sockaddr_in socket_addr;

    sock = socket(PF_INET, SOCK_STREAM, IPPROTO_SCTP);
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
    
    sock = socket(PF_INET6, SOCK_STREAM, IPPROTO_SCTP);
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
        if (args->users->name == 0 || args->users->pass == 0) {
            printf("Username: ");
            scanf("%s", name_buffer);
            printf("Password: ");
            scanf("%s", password_buffer);
        }
        else {
            strcpy(name_buffer, args->users->name);
            strcpy(name_buffer, args->users->pass);
        }
        
        int name_len = strlen(name_buffer);
        int password_len = strlen(password_buffer);

        uint8_t *message = NULL;

        message = realloc(message, 3 + name_len + password_len);
        message[0] = 0x01;
        message[1] = name_len;
        strcpy((char *)(message + 2), name_buffer);
        message[2+name_len] = password_len;
        strcpy((char *)(message + name_len + 3), password_buffer);

        send(fd, message, 3 + name_len + password_len, MSG_NOSIGNAL);
        uint8_t answer[2];

        recv(fd, answer, 2, 0);
        
        // El primer byte de la respuesta se ignora
        switch(answer[1]) {
            case 0x00:
                printf("\nSuccessful connection\n\n");
                logged_in = true;
                break;
            case 0x01:
                printf("\nIncorrect password. Please try again\n");
                exit(1);
                break;
            case 0x02:
                printf("\nNo user found. Please try again\n");
                exit(1);
                break;
            case 0x03:
                printf("\nAccess denied: user is not an admin\n");
                exit(1);
                break;
        }
    }
}


static void print_options() {
    //system("clear");
    printf("GET: \n   1: Total bytes transfered \n   2: Historical connections\n   3: Concurrent connections\n   4: List all users\n\nSET: \n   5: Add user\n   6: Add user admin\n   7: Remove user\n   8: Change user password\n   9: Enable/disable password sniffer\n\n10 - Quit\n\n");
    
}

void options(int fd){
    char selected[CREDENTIALS_SIZE];
    int select;
    while(!done){
        print_options();
        printf("Choose an option:\n");
        scanf(buff, selected);

        select = atoi(selected) -1;
        printf("\n---------------------------------------\n");
        if(select >=0 && select < CANT_OPTIONS && !done){
            option_func[select](fd);
        }
    }
}

void transfered_bytes(int fd) {
    get_command(fd, 0x00, "\nRetrieving transfered bytes...\n");
}

void connection_history(int fd) {
    get_command(fd, 0x01, "\nRetrieving connection history...\n");
}

void concurrent_connections(int fd) {
    get_command(fd, 0x02, "\nRetrieving concurrent connections...\n");
}

void retrieve_users(int fd) {
    get_command(fd, 0x03, "\nRetrieving users...\n");
}

void get_command(int fd, uint8_t cmd, char *operation) {
    printf("\n%s\n", operation);
    uint8_t request[2];
    request[0] = 0x00;
    request[1] = cmd;
    send(fd, request, 2, 0);

    uint8_t answer[1000];
    int num = 0;
    switch (cmd) {
    case 0x00:
        num = 8;
        break;
    case 0x01:
        num = 6;
        break;
    case 0x02:
        num = 6;
        break;
    case 0x03:
        num = 4;
    default:
        break;
    }
    recv(fd, answer, num, 0);

    int nargs = answer[2];

    char **result = malloc(nargs * sizeof(char *));
    if (answer[1] == 0x03) {
        get_other_users(fd, result, answer);
    }

    get_answer_handler(fd, answer, result);
}

void get_other_users(int fd, char **result, uint8_t *answer) {
    int nargs = answer[2];
    int args_len = answer[3];
    int aux = 0;
    for (int i = 0; i < nargs; i++) {
        result[i] = malloc(args_len);
        if(result[i] == NULL) {
            // nargs = i - 1;
            return;
        }
        recv(fd, result[i], args_len + 1, 0);
        aux = args_len;
        args_len = result[i][args_len];
        result[i][aux] = '\0';
    }
}

void create_user(int fd) {
    add_new_user(fd, false);
}

void create_admin(int fd) {
    add_new_user(fd, true);
}

void add_new_user(int fd, bool admin) {

    char name_buffer[CREDENTIALS_SIZE];
    char password_buffer[CREDENTIALS_SIZE];

    if (admin) {
        printf("\n---- Create admin ----\n");
    } else {
        printf("\n---- Create user ----\n");
    }
    printf("Username: ");
    scanf("%s",name_buffer);
    printf("Password: ");
    scanf("%s",password_buffer);
    printf("\n");

    int name_len = strlen(name_buffer);
    int password_len = strlen(password_buffer);

    uint8_t *message = NULL;

    message = realloc(message, 7 + name_len + password_len);
    message[0] = 0x01;
    message[1] = 0x00;
    message[2] = (admin)? 0x01 : 0x00;
    message[3] = name_len + 1;
    strcpy((char *)(message + 4), name_buffer);
    message[4 + name_len + 1] = password_len + 1;
    strcpy((char *)(message + name_len + 6), password_buffer);
    send(fd, message, 7 + name_len + password_len, MSG_NOSIGNAL);

    uint8_t answer[1];
    user_answer_handler(fd, answer);
}

void remove_user(int fd) {
    char name_buffer[CREDENTIALS_SIZE];

    printf("\n---- Delete user ----\n");
    printf("Username: ");
    scanf("%s",name_buffer);

    uint8_t *message = NULL;
    int name_len = strlen(name_buffer);

    message = realloc(message, 4 + name_len);
    message[0] = 0x01;
    message[1] = 0x01;
    message[2] = name_len + 1;
    strcpy((char *)(message + 3), name_buffer);
    
    send(fd, message, 4 + name_len, MSG_NOSIGNAL);

    uint8_t answer[1];
    user_answer_handler(fd, answer);
}

void modify_user(int fd) {
    char name_buffer[CREDENTIALS_SIZE];
    char password_buffer[CREDENTIALS_SIZE];

    printf("\n---- Modify user ----\n");
    printf("Username: ");
    scanf("%s",name_buffer);
    printf("Password: ");
    scanf("%s",password_buffer);
    printf("\n");

    uint8_t *message = NULL;

    int name_len = strlen(name_buffer);
    int password_len = strlen(password_buffer);


    message = realloc(message, 6 + name_len + password_len);
    message[0] = 0x01;
    message[1] = 0x02;
    message[2] = name_len + 1;
    strcpy((char *)(message + 3), name_buffer);
    message[3 + name_len + 1] = password_len + 1;
    strcpy((char *)(message + name_len + 5), password_buffer);

    send(fd, message, 6 + name_len + password_len, MSG_NOSIGNAL);

    uint8_t answer[1];
    user_answer_handler(fd, answer);
}

void quit(int fd) {
    uint8_t buff[] = {0x03};
    send(fd, buff, 1, 0);
    int n = recv(fd, buff, 1, 0);

    if(n != 1) {
        printf("Quit failed successfully\n");
        exit(1);
    }
    printf("Quit success\n");

    done = true;
}

void set_sniffer(int fd) {

    char confirm = 0;
    printf("\nSniffer options: \n\t1: Enable\n\t2: Disable\n");
    do {
        confirm = getchar();
    } while (confirm != '1' && confirm != '2');

    // uint8_t request[2];
    // request[0] = 0x00;
    // if(confirm == '1')
    //     request[1] = 0x00;
    // else
    //     request[1] = 0x01;
    // send(fd, request, 2, 0);

    // int n = recv(fd, request, 1, 0);

    // if(n != 1) {
    //     printf("recv failed\n");
    //     exit(1);
    // }

    // printf("Quit success");




    // resp_status status;
    // n = get_setter_result(fd, &status);

    // if(n <= 0) {
    //     exit_error();
    //     return;
    // }

    // print_response_status(status);

}

void user_answer_handler(int fd, uint8_t *answer) {
    recv(fd, answer, 1, 0);

    switch (answer[0]) {
    case 0x00:
        printf("\nOperation successfull\n");
        break;
    case 0x01:
        printf("\nServer failure\n");
        exit(1);
        break;
    case 0x02:
        printf("\nCommand type not supported\n");
        exit(1);
        break;
    case 0x03:
        printf("\nCommand not supported\n");
        exit(1);
        break;
    case 0x04:
        printf("\nError in input length\n");
        break;
    case 0x05:
        printf("\nInvalid credentials\n");
        break;
    default:
        printf("\nUnknown error\n");
        exit(1);
        break;
    }
}

void get_answer_handler(int fd, uint8_t *answer, char **result) {

    switch (answer[0]) {
    case 0x00:
        printf("\nOperation successfull\n");
        break;
    case 0x01:
        printf("\nServer failure\n");
        exit(1);
        break;
    case 0x02:
        printf("\nCommand type not supported\n");
        exit(1);
        break;
    case 0x03:
        printf("\nCommand not supported\n");
        exit(1);
        break;
    default:
        printf("\nUnknown error\n");
        exit(1);
        break;
    }

    unsigned long bytes;
    if(answer[1] == 0x00) {
        bytes = (answer[4] << 24) | (answer[5] << 16) | (answer[6] << 8) | answer[7];
        printf("\nBytes transfered: %lu\n", bytes);
    } else if (answer[1] == 0x01) {
        bytes = (answer[4] << 8) | answer[5];
        printf("\nHistorical connections: %lu\n", bytes);
    } else if (answer[1] == 0x02) {
        bytes = (answer[4] << 8) | answer[5];
        printf("\nConcurrent connections: %lu\n", bytes);
    } else if(answer[1] == 0x03) {
        printf("\n ---- Users ----\n");
        for (int i = 0; i < answer[2]; i++) {
            printf("%s\n", result[i]);
        }
    }
    putchar('\n');
}







// 0-create_user 1-create_admin 2-delete_user 3-modify_user
// void user_command(int fd, uint8_t command) {
//     char name_buffer[CREDENTIALS_SIZE];
//     char password_buffer[CREDENTIALS_SIZE];
//     uint8_t cmd;

//     switch (command) {
//     case 0:
//         printf("\n---- Create user ----\n");
//         cmd = 0x01;
//         break;
//     case 1:
//         printf("\n---- Create admin ----\n");
//         cmd = 0x01;
//         break;
//     case 2:
//         printf("\n---- Delete user ----\n");
//         cmd = 0x02;
//         break;
//     case 3:
//         printf("\n---- Modify user ----\n");
//         cmd = 0x03;
//         break;
//     default:
//         printf("\nUnknown error\n");
//         exit(1);
//         break;
//     }
//     printf("Username: ");
//     scanf("%s",name_buffer);
//     if (command != 2) {
//         printf("Password: ");
//         scanf("%s",password_buffer);
//         char password_len = strlen(password_buffer);
//     }
//     printf("\n");

//     int nbyte = 0;
//     uint8_t *message = NULL;
//     int name_len = strlen(name_buffer);
//     int password_len = strlen(password_buffer);
//     message = realloc(message, 3 + name_len + password_len);
//     message[nbyte++] = 0x01;
//     message[nbyte++] = cmd;

//     if (command == 0 || command == 1) {
//         message[nbyte++] = (command == 1)? 0x00 : 0x01;
//     }

//     message[nbyte++] = strlen(name_buffer);
//     strcpy((char *)(message + nbyte++), name_buffer);

//     if (command != 2) {
//         message[(nbyte++) + name_len] = password_len;
//         strcpy((char *)(message + (nbyte++) + name_len), password_buffer);
//     }

//     send(fd, message, strlen(message), MSG_NOSIGNAL);

//     uint8_t answer[1];
//     user_answer_handler(fd, answer);
// }