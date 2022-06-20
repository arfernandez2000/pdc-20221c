#include "../include/stadistics.h"

Stadistics stadistics;

void stadistics_init(){
    memset(&stadistics, 0, sizeof(stadistics));
}

void stadistics_increase_concurrent(){
    if(stadistics.concurrentConnections < UINT16_MAX)
        stadistics.concurrentConnections++;
    if(stadistics.totalConnections < UINT16_MAX)
        stadistics.totalConnections++;
}

void stadistics_decrease_concurrent(){
    if(stadistics.concurrentConnections > 0)
        stadistics.concurrentConnections--;
}

void stadistics_increase_bytes_sent(uint64_t bytes){
    if(stadistics.bytesSent + bytes < UINT32_MAX)
        stadistics.bytesSent+=bytes;
}

uint16_t get_concurrent_connections(){
    return stadistics.concurrentConnections;
}

uint16_t get_total_connections(){
    return stadistics.totalConnections;
}

uint32_t get_bytes_sent(){
    return stadistics.bytesSent;
}


