#include "stadistics.h"

Stadistics stadistics;

void stadistics_init(){
    memset(&stadistics, 0, sizeof(stadistics));
}

void stadistics_increase_concurrent(){
    if(stadistics.totalConnections < UINT64_MAX)
        stadistics.totalConnections++;
    if(stadistics.concurrentConnections < UINT16_MAX)
        stadistics.concurrentConnections++;
}

void stadistics_decrease_concurrent(){
    if(stadistics.concurrentConnections > 0)
        stadistics.concurrentConnections--;
}

void stadistics_increase_bytes_sent(uint64_t bytes){
    if(stadistics.bytesSent + bytes < UINT64_MAX)
        stadistics.bytesSent+=bytes;
}

void stadistics_increase_bytes_received(uint64_t bytes){
    if(stadistics.bytesReceived + bytes < UINT64_MAX)
        stadistics.bytesReceived+=bytes;
}

