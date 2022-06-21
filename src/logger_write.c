#include "../include/logger_write.h"

#include <stdlib.h>

logger * data = NULL;

logger * get_data_logger(){
    return data;
}

int init_logger(fd_selector selector){
    data = malloc(sizeof(*data));
    if(data == NULL){
        return -1;
    }
    data->selector = selector;
    buffer_init(&data->write_buff,(sizeof((data->raw_buff))/sizeof(((data->raw_buff))[0])), data->raw_buff);
    return 1;
}

void free_logger(){
    if(data != NULL){
        free(data);
    }
}


void write_logger(struct selector_key * event){
    logger * w_logger = (logger *)event->data;
    
    size_t size;
    buffer * buff = &w_logger->write_buff;
    uint8_t *ptr = buffer_read_ptr(buff, &size);
    size_t n = write(1, ptr,size);
    if(n > 0){
        if((unsigned)n < size){
            buffer_read_adv(buff,n);
        }
        else{
            buffer_read_adv(buff,size);
            selector_set_interest_key(event, OP_NOOP);
        }
    }
}

