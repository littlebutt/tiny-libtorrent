#include "tcp.h"


int tcp_connect(const char *ip, int port)
{
    int sock;
    struct sockaddr_in server_addr;

#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printf("[tcp] WSAStartup failed\n");
        return 0;
    }
#endif

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        printf("[tcp] Socket creation failed\n");
        return 0;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0)
    {
        printf("[tcp] Invalid address/ Address not supported\n");
#ifdef _WIN32
        closesocket(sock);
        WSACleanup();
#else
            close(sock);
#endif
        return 0;
    }

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        printf("[tcp] Connection failed");
#ifdef _WIN32
        printf(" %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
#else
        printf(" %d\n", errno);
        close(sock);
#endif
        return 0;
    }

    printf("[tcp] Connected to server %s on port %d\n", ip, port);

    return sock;
}

void tcp_close(int sock)
{
#ifdef _WIN32
    closesocket(sock);
    WSACleanup();
#else
    close(sock);
#endif
}

int tcp_send(int sock, const char *msg, size_t msglen, char **recvs)
{
    send(sock, msg, msglen, 0);
    printf("[tcp] Sent: %s\n", msg);

    size_t buflen = 1024;
    char *buf = (char *)malloc(buflen);
    int bufsize;
    if ((bufsize = recv(sock, buf, buflen, 0)) < 0)
    {
#ifdef _WIN32
        printf("[tcp] Fail to recv message from peer: %d", WSAGetLastError());
#else
        printf("[tcp] Fail to recv message from peer: %d", errno);
#endif
        // buflen = buflen << 1;
        // buf = (char *)realloc(buf, buflen);
        // if (buf == NULL)
        // {
        //     return 0;
        // }
        return 0;
    }
    *recvs = buf;
    return bufsize;
}