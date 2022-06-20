#ifndef LOGGER_WRITE_H
#define LOGGER_WRITE_H
#include <stdint.h>
#include "selector.h"
#include "buffer.h"
#define MAX_BUFF 2048

typedef struct logger{
    uint8_t raw_buff[MAX_BUFF];
    buffer write_buff;
    fd_selector selector;
}logger;

void write_logger(struct selector_key * key);
int init_logger(fd_selector selector);
void free_logger();
struct logger * get_data_logger();
#endif

