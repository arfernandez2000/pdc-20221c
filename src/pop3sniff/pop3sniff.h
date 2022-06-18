#ifndef POP3SNIFF_H
#define POP3SNIFF_H

#include <stdint.h>
#include "../buffer/buffer.h"

#define MAX_SIZE_USER 255
#define MAX_BUFFER_POP3_SIZE 4096

enum pop3sniff_st
{
    POP3_INITIAL,
    POP3_USER,
    POP3_READ_USER, 
    POP3_PASSWORD,
    POP3_READ_PASSWORD, 
    POP3_CHECK,
    POP3_SUCCESS,
    POP3_DONE
};

struct pop3_sniff{
    enum pop3sniff_st state;

    buffer buffer;
    uint8_t raw_buff[MAX_BUFFER_POP3_SIZE];
    char username[MAX_SIZE_USER];
    char password[MAX_SIZE_USER];
    uint8_t read;
    uint8_t remaining;
    uint8_t check_read;
    uint8_t check_remaining;
    bool parsing;
};

void pop3_init(struct pop3_sniff* sniff);

enum pop3sniff_st pop3_parse(struct pop3_sniff* sniff,uint8_t b);

bool pop3_done(struct pop3_sniff *sniff);

bool pop3_parsing(struct pop3_sniff *sniff);

enum pop3sniff_st pop3_consume(struct pop3_sniff *sniff);

#endif


