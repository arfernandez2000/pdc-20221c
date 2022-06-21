#include <string.h>
#include <ctype.h>
#include "../include/pop3sniff.h"
#include "../include/register.h"

#define INITIAL_SIZE 50

static const char * OK = "+OK";
static const char * USER = "USER ";
static const char * PASSWORD = "PASS ";
static const char * ERROR_SNIFF = "-ERR";

static void reset(struct pop3_sniff* sniff, uint8_t left){
    sniff->read = 0;
    sniff->remaining = left;
}

void pop3_init(struct pop3_sniff* sniff){
    sniff->state = POP3_INITIAL;
    memset(sniff->raw_buff,0,MAX_BUFFER_POP3_SIZE);
    buffer_init(&sniff->buffer, sizeof(sniff->raw_buff)/sizeof(sniff->raw_buff[0]), sniff->raw_buff);
    sniff->read = 0;
    sniff->remaining = strlen(OK);
    sniff->parsing = true;
}

static enum pop3sniff_st initial_msg(struct pop3_sniff* sniff,uint8_t b){
    enum pop3sniff_st state = POP3_INITIAL;
    if(tolower(b) == tolower(*(OK + sniff->read))){
        sniff->read++;
        sniff->remaining--;
        if(sniff->remaining == 0){
            state = POP3_USER;
            sniff->read = 0;
            sniff->remaining = strlen(USER);
        }
    }
    else{
        state = POP3_DONE;
    }
    return state;
}

enum pop3sniff_st sniffer(struct pop3_sniff * sniff,uint8_t b, const char * sniffing,enum pop3sniff_st initial_state, enum pop3sniff_st next_state){
    enum pop3sniff_st state = initial_state;
    if(tolower(b) == tolower(*(sniffing + sniff->read))){
        sniff->read++;
        sniff->remaining--;
        if(sniff->remaining == 0){
            sniff->read = 0;
            state = next_state;
        }        
    }
    else{
        if(sniff->read != 0){
            reset(sniff,strlen(sniffing));
        }
    } 
    return state;
}

enum pop3sniff_st read_user(struct pop3_sniff* sniff,uint8_t b){
    enum pop3sniff_st state = POP3_READ_USER;
    if(b != '\n'){
        if(sniff->read < MAX_SIZE_USER){
            sniff->username[sniff->read++] = b;
        }
    }
    else{
        sniff->username[sniff->read] = '\0';
        sniff->read = 0;
        sniff->remaining = strlen(PASSWORD);
        sniff->check_read = 0;
        sniff->check_remaining = strlen(ERROR_SNIFF);
        state = POP3_PASSWORD;
}
    return state;
}

enum pop3sniff_st read_password(struct pop3_sniff* sniff,uint8_t b){
    enum pop3sniff_st state = POP3_READ_PASSWORD;
    if(b != '\n'){
        if(sniff->read < MAX_SIZE_USER){
            sniff->password[sniff->read++] = b;
        }
    }
    else{
        sniff->password[sniff->read] = '\0';
        sniff->read = 0;
        sniff->check_read = 0;
        state = POP3_CHECK; 
}
    return state;
}

enum pop3sniff_st check(struct pop3_sniff* sniff,uint8_t b){
    enum pop3sniff_st state = POP3_CHECK;
    if(tolower(b) == tolower(*(OK + sniff->read))){
        sniff->read++;
        if(sniff->read == strlen(OK)){
            state = POP3_SUCCESS;
        }
    }
    else if(tolower(b) == tolower(*(ERROR_SNIFF + sniff->check_read))){
        sniff->check_read++;
        if(sniff->check_read == strlen(ERROR_SNIFF)){
            state = POP3_USER;
        }
    }
    fprintf(stdout, "saliendo de check con %d\n", state);
    return state;
}

enum pop3sniff_st pop3_parse(struct pop3_sniff* sniff,uint8_t b){
    switch (sniff->state)
    {
    case POP3_INITIAL:
        sniff->state = initial_msg(sniff,b);
        break;
    case POP3_USER:
        sniff->state = sniffer(sniff,b,USER,POP3_USER, POP3_READ_USER);
        break;
    case POP3_READ_USER:
        sniff->state = read_user(sniff,b);
        break;
    case POP3_PASSWORD:
        sniff->state = sniffer(sniff,b,PASSWORD,POP3_PASSWORD, POP3_READ_PASSWORD);
        break;
    case POP3_READ_PASSWORD:
        sniff->state = read_password(sniff,b);
        break;
    case POP3_CHECK:
        sniff->state = check(sniff,b);
        break;
    default:
        break;
    }
    return sniff->state;
}

bool pop3_done(struct pop3_sniff *sniff){
    return sniff->state == POP3_DONE || sniff->state == POP3_SUCCESS;
}

bool pop3_parsing(struct pop3_sniff *sniff){
    return sniff->parsing;
}

enum pop3sniff_st pop3_consume(struct pop3_sniff *sniff, register_st *logger){

    while(buffer_can_read(&sniff->buffer) && !pop3_done(sniff)){
        uint8_t b = buffer_read(&sniff->buffer);
        pop3_parse(sniff,b);
    }
    fprintf(stdout, "despues del while\n");
    if (sniff->state == POP3_SUCCESS) {
        fprintf(stdout, "en success\n");
        strcpy(logger->user, sniff->username);
        strcpy(logger->passwd, sniff->password);
        fprintf(stdout, "bye success\n");
        //logger->user = sniff->username;
        //logger->passwd = sniff->password;
        log_sniff(logger);
    }
    return sniff->state;
}