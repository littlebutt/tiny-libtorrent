#include "tcp.h"


int _set_socket_nonblocking(int sock)
{
#ifdef _WIN32
    unsigned long mode = 1;
    return ioctlsocket(sock, FIONBIO, &mode);
#else
    int flags = fcntl(sock, F_GETFL, 0);
    return fcntl(sock, F_SETFL, flags | O_NONBLOCK);
#endif
}

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

    if (_set_socket_nonblocking(sock) < 0)
    {
        printf("[tcp] Failed to set non-blocking mode\n");
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
#ifdef _WIN32
        if (WSAGetLastError() != WSAEWOULDBLOCK)
        {
            printf("[tcp] Connection failed");
            printf(" %d\n", WSAGetLastError());
            closesocket(sock);
            WSACleanup();
#else
        if (errno != EINPROGRESS)
        {
            printf("[tcp] Connection failed");
            printf(" %d\n", errno);
            close(sock);
#endif
            return 0;
        }
    }

    fd_set writefds;
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;

    FD_ZERO(&writefds);
    FD_SET(sock, &writefds);

    int select_result = select(sock + 1, NULL, &writefds, NULL, &timeout);
    if (select_result > 0)
    {
        int so_error;
        socklen_t len = sizeof(so_error);
        getsockopt(sock, SOL_SOCKET, SO_ERROR, (void *)&so_error, &len);
        if (so_error == 0) {
            printf("[tcp] Connected to server %s on port %d\n", ip, port);
            return sock;
        } else {
            printf("[tcp] Connection failed: %d\n", so_error);
            return 0;
        }
    } else if (select_result == 0) {
        printf("[tcp] Connection timed out %s:%d\n", ip, port);
        return 0;
    } else {
        printf("[tcp] Select error\n");
        return 0;
    }
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


int wait_for_socket(int sock, int for_write, int timeout)
{
    fd_set fds;
    struct timeval tv;

    FD_ZERO(&fds);
    FD_SET(sock, &fds);

    tv.tv_sec = timeout;
    tv.tv_usec = 0;

    if (for_write) {
        return select(sock + 1, NULL, &fds, NULL, &tv);
    } else {
        return select(sock + 1, &fds, NULL, NULL, &tv);
    }
}


int tcp_send(int sock, const char *msg, size_t msglen, char **recvs)
{
    send(sock, msg, msglen, 0);
    dbg_bin("[tcp] Sent", msg, msglen);
    

    size_t buflen = 1024;
    char *buf = (char *)malloc(buflen);
    if (buf == NULL)
    {
        return 0;
    }
    memset(buf, 0, buflen);
    int bufsize;
    while ((bufsize = recv(sock, buf, buflen, 0)) != 0)
    {
        if (bufsize < 0)
        {
#ifdef _WIN32
            if (WSAGetLastError() == WSAEWOULDBLOCK)
#else
            if (errno == EWOULDBLOCK)
#endif
            {
                if (wait_for_socket(sock, 0, 5) > 0)
                {
                    bufsize = recv(sock, buf, buflen, 0);
                    if (bufsize < buflen)
                    {
                        break;
                    }
                    else
                    {
                        buflen = buflen << 1;
                        buf = (char *)realloc(buf, buflen);
                        if (buf == NULL)
                        {
                            return 0;
                        }
                    }
                }
                else
                {
                    printf("[tcp] Receive timeout\n");
                    return 0;
                }
            }
            else
            {
#ifdef _WIN32
                printf("[tcp] Fail to recv message from peer: %d", WSAGetLastError());
#else
                printf("[tcp] Fail to recv message from peer: %d", errno);
#endif
                return 0;
            }
        }
        else if (bufsize < buflen)
        {
            break;
        }
        else
        {
            buflen = buflen << 1;
            buf = (char *)realloc(buf, buflen);
            if (buf == NULL)
            {
                return 0;
            }
        }
    }
    *recvs = buf;
    return bufsize;
}