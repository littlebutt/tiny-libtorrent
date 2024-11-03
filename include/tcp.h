#ifndef TCP_H
#define TCP_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #ifdef _MSC_VER
        #pragma comment(lib, "Ws2_32.lib")
    #endif // _MSC_VER
#else
    #include <arpa/inet.h>
    #include <netinet/in.h>
    #include <unistd.h>

#endif

#include "utils.h"

int tcp_connect(const char *ip, int port);

void tcp_close(int sock);

int tcp_send(int sock, const char *msg, size_t msglen, char **recvs);

int tcp_send_for_message(int sock, const char *msg, size_t msglen, char **recvs);

#endif // TCP_H