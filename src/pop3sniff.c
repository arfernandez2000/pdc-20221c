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
    reset(sniff, strlen(OK));
}
static enum pop3sniff_st initial_msg(struct pop3_sniff* sniff,uint8_t b){
    if(tolower(b) == tolower(*(OK + sniff->read))){
        sniff->read++;
        sniff->remaining--;
        if(sniff->remaining == 0){
            sniffer->state = POP3_USER;
            sniff->read = 0;
            sniff->remaining = strlen(USER);
        }
    }
    else{
        sniffer->state = POP3_DONE;
    }
     return sniffer->state;
 }

enum pop3sniff_st user_sniff(struct pop3_sniff * sniff,uint8_t b, const char * sniffing,enum pop3sniff_st initial_state, enum pop3sniff_st next_state){
     if(tolower(b) == tolower(*(sniffing + sniff->read))){
         sniff->read++;
         sniff->remaining--;
         if(sniff->remaining == 0){
             sniff->read = 0;
             sniff->state = POP3_READ_USER;
         }        
     }
     else{
        if(sniff->read != 0){
            reset(sniff,strlen(USER));
        }
    } 
    return sniff->state;
}
enum pop3sniff_st read_user(struct pop3_sniff* sniff,uint8_t b){
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
        sniff->state = POP3_PASSWORD;
}
    return sniff->state;
}

enum pop3_sniffer_state password_sniff(struct pop3_sniffer* s,uint8_t b){
  if(toupper(b) == toupper(*(PASS + s -> read))) {
    s -> read++;
    s -> remaining--;
    if(s -> remaining == 0) {
      s -> read = 0;
      s -> state = POP3_READ_PASSWORD;
    }        
  }
  else {
    if(s -> read != 0) {
      reset_read(s,strlen(PASSWORD));
    }
  }

  return s -> state;
}


enum pop3sniff_st read_password(struct pop3_sniff* sniff,uint8_t b){
    if(b != '\n'){
        if(sniff->read < MAX_SIZE_USER){
            sniff->password[sniff->read++] = b;
        }
    }
    else{
        sniff->password[sniff->read] = '\0';
        sniff->read = 0;
        sniff->check_read = 0;
        sniff->state = POP3_CHECK; 
}
    return sniff->state;
}
enum pop3sniff_st check(struct pop3_sniff* sniff,uint8_t b){
    if(tolower(b) == tolower(*(OK + sniff->read))){
        sniff->read++;
        if(sniff->read == strlen(OK)){
            sniff->state = POP3_PASSWORD;
        }
    }
    else if(tolower(b) == tolower(*(ERROR_SNIFF + sniff->check_read))){
        sniff->check_read++;
        if(sniff->check_read == strlen(ERROR_SNIFF)){
             sniff->state = POP3_USER;
         }
     }
     fprintf(stdout, "saliendo de check con %d\n", state);
     return sniff->state;
 }
 enum pop3sniff_st check_password(struct pop3_sniff* sniff,uint8_t b){
    if(tolower(b) == tolower(*(OK + sniff->read))){
        sniff->read++;
        if(sniff->read == strlen(OK)){
            sniff->state = POP3_DONE;
        }
    }
    else if(tolower(b) == tolower(*(ERROR_SNIFF + sniff->check_read))){
        sniff->check_read++;
        if(sniff->check_read == strlen(ERROR_SNIFF)){
             sniff->state = POP3_USER;
         }
     }
     fprintf(stdout, "saliendo de check con %d\n", state);
     return sniff->state;
 }

enum pop3sniff_st pop3_parse(struct pop3_sniff* sniff,uint8_t b){
    switch (sniff->state)
    {
    case POP3_INITIAL:
         sniff->state = initial_msg(sniff,b);
         break;
     case POP3_USER:
         sniff->state = user_sniff(sniff,b);
         break;
     case POP3_READ_USER:
         sniff->state = read_user(sniff,b);
         break;
     case POP3_CHECK_USER:
        sniff->state = check(sniff, b);
     case POP3_PASSWORD:
         sniff->state = password_sniff(sniff,b);
         break;
     case POP3_READ_PASSWORD:
         sniff->state = read_password(sniff,b);
        break;
    case POP3_CHECK:
        sniff->state = check_password(sniff,b);
        break;
    case POP3_DONE:
    case POP3_ERROR:
    break;
    default:
        break;
    }
    return sniff->state;
}
bool pop3_done(struct pop3_sniff *sniff){
    return sniff->state == POP3_DONE || sniff->state == POP3;
}
bool pop3_parsing(struct pop3_sniff *sniff){
    return sniff->state > pop3_init && sniff->state < pop3_done;
}
enum pop3sniff_st pop3_consume(struct pop3_sniff *sniff, void * session){
    while(buffer_can_read(&sniff->buffer) && !pop3_done(sniff)){
         uint8_t b = buffer_read(&sniff->buffer);
         pop3_parse(sniff,b);
     }
    pop3_sniff *sniffinfo = malloc (sizeof (pop3_sniff));
    if(sniffinfo == NULL){
        return POP3_ERROR
    }
    strcpy(sniffinfo->password, sniff->password);
    strcpy(sniffinfo->username, sniff->username);
    sniffinfo->server_len = &(((Session *) session)->server.address_len);
    sniffinfo->addr = &(((Session *) session)->server.address);
    log_sniff(sniffinfo, session);
    log_sniff(sniffinfo, session);
    insert(list,sniffinfo);
    return sniff->state;
}