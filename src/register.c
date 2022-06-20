#include "../include/register.h"
#include <string.h>
#include <stdio.h>
#include "../include/socks5.h"
#include "../include/logger_write.h"

#include <sys/types.h>

static void date_to_string(char * date){
    
    time_t timer = time(NULL);
    struct tm * tm = gmtime(&timer);
    strftime(date, DATE_SIZE, "%Y-%m-%dT%TZ",tm);
}


static const char * ip_to_string(struct sockaddr_storage addr,char *ret,int length){
    if(addr.ss_family == AF_INET){
        return inet_ntop(addr.ss_family, &(((struct sockaddr_in *)&addr)->sin_addr), ret, length);

    }
    else{
        return inet_ntop(addr.ss_family, &(((struct sockaddr_in6 *)&addr)->sin6_addr), ret, length);
    }
}

static char* user_to_string(register_st * socks_info){
    if(socks_info->method == METHOD_NO_AUTHENTICATION_REQUIRED){
        return "----";
    }
    else{
        return (char*) socks_info->user_info.username;
    }
}

static in_port_t addr_port(struct sockaddr_storage addr){
    return addr.ss_family == AF_INET ? ((struct sockaddr_in*)&addr)->sin_port : ((struct sockaddr_in6*)&addr)->sin6_port;
}

static char* dest_addr_to_string(register_st *socks_info){
    char *ip = NULL; 
    switch (socks_info->atyp)
    {
    case socks_addr_type_ipv4:
        ip = (char *)malloc(INET_ADDRSTRLEN * sizeof(char));
        inet_ntop(AF_INET,&socks_info->dest_addr.ipv4.sin_addr, ip, INET_ADDRSTRLEN);
        break;
    case socks_addr_type_ipv6:
        ip = (char *)malloc(INET6_ADDRSTRLEN * sizeof(char));
        inet_ntop(AF_INET6,&socks_info->dest_addr.ipv6.sin6_addr, ip, INET6_ADDRSTRLEN);
        break;
    case socks_addr_type_domain:
        ip = (char *)malloc((strlen(socks_info->dest_addr.fdqn)+1) * sizeof(char));
        strcpy(ip,socks_info->dest_addr.fdqn);
        break;
    }
    return ip;
}

static void print_log(register_st *register_info, char type) {
    char date[DATE_SIZE];
    date_to_string(date);
    int length = register_info->client_addr.ss_family == AF_INET ? INET_ADDRSTRLEN : INET6_ADDRSTRLEN;
    char ret[length];
    ip_to_string(register_info->client_addr, ret,length);
    char * dest_ip = dest_addr_to_string(register_info);
    char *print = NULL;
    size_t count;
    logger * write_data = get_data_logger();
    uint8_t * ptr = buffer_write_ptr(&write_data->write_buff,&count);
    int n = 0;
    if(type == 'A') {
        print = "[%s]\t%s\tA\t%s\t%u\t%s\t%u\tstatus=%d\n";
        n = snprintf((char*)ptr,count,print, date, user_to_string(register_info), ret, ntohs(addr_port(register_info->client_addr)), dest_ip, ntohs(register_info->dest_port),register_info->status);
    }
    else if(type == 'P') {
        print = "[%s]\t%s\tPOP3\t%s\t%u\t%s\t%s\n";
        n = snprintf((char*)ptr,count,print, date, user_to_string(register_info), dest_ip, ntohs(register_info->dest_port), register_info->user, register_info->passwd);
    }
    if(n < 0){
        // Error en la copia
    }
    if ((unsigned)n > count){
        buffer_write_adv(&write_data->write_buff, count);
    }
    else{
        buffer_write_adv(&write_data->write_buff,n);
    }
    selector_set_interest(write_data->selector,1, OP_WRITE);
    free(dest_ip);
}

void log_access(register_st *register_info){
    print_log(register_info, 'A');
}
// config en runtime -> cambiar tamaño de buffers

void log_sniff(register_st *register_info) {
    print_log(register_info, 'P');
}
