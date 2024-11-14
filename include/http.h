#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
    #include <winsock2.h>
    #ifdef _MSC_VER
        #pragma comment(lib, "Ws2_32.lib")
    #endif // _MSC_VER
#else
    #include <arpa/inet.h>
    #include <netdb.h> // gethostbyname for Linux/Mac
    #include <sys/socket.h>
    #include <unistd.h> // close for Linux/Mac
#endif                  // _WIN32

int http_get(const char *url, char **recvs);

char *http_url_encode(const char *url, int urllen);

char *http_response_header(const char *recvs, const char *header);

char *http_response_body(const char *recvs, size_t content_length);
