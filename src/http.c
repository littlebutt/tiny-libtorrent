#include "http.h"

void _init_sockets()
{
#ifdef _WIN32
    WSADATA wsa;
    printf("[http] Initializing Winsock...\n");
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        printf("[http] Failed. Error Code: %d\n", WSAGetLastError());
        exit(1);
    }
    printf("[http] Winsock initialized.\n");
#endif
}

void _cleanup_sockets()
{
#ifdef _WIN32
    WSACleanup();
#endif
}

char * _build_message(const char *hostname, const char *path, const char *params)
{
    char *message = (char *)malloc(sizeof(char) * 8192);
    if (message == NULL)
    {
        return NULL;
    }
    if (params != NULL)
    {
        sprintf(message, "GET %s?%s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", path, params, hostname);
    }
    else
    {
        sprintf(message, "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", path, hostname);
    }
    
    return message;
}

char * _get_ip_from_url(const char *url)
{
    struct hostent *he;
    struct in_addr **addr_list;
    char *ip = (char *)malloc(sizeof(char) * 1024);

    if ((he = gethostbyname(url)) == NULL)
    {
#ifdef _WIN32
        printf("[http] gethostbyname failed. Error: %d\n", WSAGetLastError());
#else
        printf("[http] gethostbyname failed. Error: %d\n", h_errno);
#endif
        return NULL;
    }

    addr_list = (struct in_addr **)he->h_addr_list;
    strcpy(ip, inet_ntoa(*addr_list[0]));

    return ip;
}

int _parse_url(const char *url, char **hostname, char **port, char **path, char **params)
{
    char *url_copy = strdup(url);
    char *url_copy2 = strdup(url);

    char *colon_ptr = strchr(url_copy, ':');
    char *slash_ptr = strchr(url_copy, '/');
    char *slash_ptr2 = strchr(url_copy2, '/');
    char *question_ptr = strchr(url_copy2, '?');

    if (colon_ptr && (!slash_ptr || colon_ptr < slash_ptr))
    {
        *colon_ptr = '\0';
        *hostname = strdup(url_copy);
        if (slash_ptr)
        {
            *slash_ptr = '\0';
        }
        *port = strdup(colon_ptr + 1);
    }
    else
    {
        if (slash_ptr)
        {
            *slash_ptr = '\0';
        }
        *hostname = strdup(url_copy);  // host
        *port = strdup("80");
    }

    if (slash_ptr2)
    {
        if (question_ptr)
        {
            *question_ptr = '\0';
            *path = strdup(slash_ptr2);
            *params = strdup(question_ptr + 1);
        } else
        {
            *path = strdup(slash_ptr2);
            *params = NULL;
        }
    } else
    {
        *path = strdup("/");
        *params = NULL;
    }

    free(url_copy);
    free(url_copy2);
    return 1;
}


int http_get(const char *url, char **recvs)
{
    int sock, res;
    struct sockaddr_in server;
    char *message, *server_reply;
    size_t server_reply_size;

    char *hostname, *path, *params;
    char *port;

    server_reply_size = 4096;
    server_reply = (char *)malloc(sizeof(char) * server_reply_size);
    if (server_reply == NULL)
    {
        return 0;
    }
    memset(server_reply, 0, 4096);

    _init_sockets();

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        printf("[http] Could not create socket\n");
        free(server_reply);
        return 0;
    }
    printf("[http] Socket created.\n");

    res = _parse_url(url, &hostname, &port, &path, &params);
    if (!res)
    {
        free(server_reply);
        return 0;
    }

    char *ip = _get_ip_from_url(hostname);
    if (ip == NULL)
    {
        free(server_reply);
        return 0;
    }

    server.sin_addr.s_addr = inet_addr(ip);
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(port));

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        free(server_reply);
#ifdef _WIN32
        printf("[http] connect failed. Error: %d\n", WSAGetLastError());
#else
        printf("[http] connect failed. Error: %d\n", h_errno);
#endif
        return 0;
    }
    printf("[http] Connected to server.\n");

    message = _build_message(hostname, path, params);
    if (message == NULL)
    {
        free(server_reply);
    }

    if (send(sock, message, strlen(message), 0) < 0)
    {
        printf("[http] Send failed\n");
        free(server_reply);
        free(message);
        return 0;
    }
    printf("[http] Data sent.\n");

    int bytes_received = 0;
    int total_bytes = 0;
    do
    {
        bytes_received = recv(sock, server_reply + total_bytes, 1024, 0);
        if (bytes_received < 0)
        {
#ifdef _WIN32
            printf("[http] recv failed. Error: %d\n", WSAGetLastError());
#else
            printf("[http] recv failed. Error: %d\n", h_errno);
#endif
            free(server_reply);
            free(message);
#ifdef _WIN32
            closesocket(sock);
#else
            close(sock);
#endif
            return 0;
        }

        total_bytes += bytes_received;

        if (total_bytes + 1024 >= server_reply_size)
        {
            server_reply_size *= 2;
            char *new_response = realloc(server_reply, server_reply_size);
            if (new_response == NULL)
            {
                printf("[http] Failed to reallocate memory");
                free(server_reply);
                free(message);
                return 0;
            }
            server_reply = new_response;
        }

    } while(bytes_received > 0);

    server_reply[total_bytes + 4096] = '\0';

    *recvs = server_reply;

#ifdef _WIN32
    closesocket(sock);
#else
    close(sock);
#endif

    _cleanup_sockets();
    free(message);
    return 1;
}