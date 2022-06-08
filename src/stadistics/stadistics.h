#ifndef STADISTICS_H
#define STADISTICS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Stadistics{
    uint64_t totalConnections;
    uint16_t concurrentConnections;

    uint64_t bytesSent;
    uint64_t bytesReceived;
}Stadistics;

void stadistics_init(void);
void stadistics_increase_concurrent(void);
void stadistics_decrease_concurrent();
void stadistics_increase_bytes_sent(uint64_t bytes);
void stadistics_increase_bytes_received(uint64_t bytes);
