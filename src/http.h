#ifndef HTTP_H
#define HTTP_H

#include <stdio.h>
#include <string.h>
#ifdef _WIN32
    #include <winsock2.h>
    #ifdef _MSC_VER
        #pragma comment(lib, "Ws2_32.lib")
    #endif // _MSC_VER
#else
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <netdb.h>  // gethostbyname for Linux/Mac
    #include <unistd.h> // close for Linux/Mac
#endif // _WIN32


int http_get(const char *url, char **recvs);

#endif // HTTP_H