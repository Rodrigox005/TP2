#pragma once

#include <stdlib.h>
#include <arpa/inet.h>

void loexit(const char *msg); 

int addrparse(const char *addrstr, const  char *portstr,
            struct sockaddr_storage *storage);

void addrtostr(const struct sockaddr *addr, char *str, size_t strsize);

int server_sockaddr_init(const char *prot,const char *portstr,struct sockaddr_storage *storage);
