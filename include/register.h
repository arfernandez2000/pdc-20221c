#ifndef REGISTER_H
#define REGISTER_H

#include "socks5utils.h"


#define DATE_SIZE 21


void log_access(register_st *socks_info);
void log_sniff(pop3_sniff * sniff,register_st* socks_info);

#endif
