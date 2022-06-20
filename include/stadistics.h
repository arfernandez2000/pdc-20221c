#ifndef STADISTICS_H
#define STADISTICS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef struct Stadistics{
    uint16_t totalConnections;
    uint16_t concurrentConnections;

    uint32_t bytesSent;
}Stadistics;

void stadistics_init(void);
uint16_t get_concurrent_connections();
uint16_t get_total_connections();
uint32_t get_bytes_sent();
void stadistics_increase_concurrent(void);
void stadistics_decrease_concurrent();
void stadistics_increase_bytes_sent(uint64_t bytes);

#endif