#ifndef STADISTICS_H
#define STADISTICS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef struct Stadistics{
    uint64_t totalConnections;
    uint16_t concurrentConnections;

    uint64_t bytesSent;
}Stadistics;

void stadistics_init(void);
uint16_t get_concurrent_connections();
uint64_t get_total_connections();
uint64_t get_bytes_sent();
void stadistics_increase_concurrent(void);
void stadistics_decrease_concurrent();
void stadistics_increase_bytes_sent(uint64_t bytes);

#endif