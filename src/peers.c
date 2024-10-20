#include "peers.h"

size_t _read_peers(char **peers, const char *buf, size_t buflen)
{
    size_t peerslen = 0;
    struct bencode ctx[1];
    int res = 0;
    // A flag for memoizing keys in bencode:
    // 1 for peers
    int flag = 0;
    bencode_init(ctx, buf, buflen);
    // XXX: Assume all data we need is in a dict.
    while ((res = bencode_next(ctx)) > 0)
    {
        switch (res)
        {
            case BENCODE_STRING:
            {
                char *key = (char *)malloc((ctx->toklen + 1) * sizeof(char));
                if (key == NULL)
                {
                    return 0;
                }
                memcpy(key, ctx->tok, ctx->toklen);
                key[ctx->toklen] = '\0';
                switch (flag)
                {
                    case 0:
                    {
                        if (strcmp(key, "peers") == 0)
                        {
                            flag = 1;
                        }
                        break;
                    }
                    case 1:
                    {
                        flag = 0;
                        char *value = (char *)malloc(ctx->toklen * sizeof(char));
                        if (value == NULL) {
                            return 0;
                        }
                        memcpy(value, key, ctx->toklen * sizeof(char));
                        *peers = value;
                        peerslen = ctx->toklen;
                        break;   
                    }
                }
                free(key);
            }
        }
    }
    bencode_free(ctx);
    return peerslen;    
}

char * _build_ip(char *buf)
{
    uint8_t ip[4];
    char *res;
    for (int i = 0; i < 4; i++)
    {
        ip[i] = (uint8_t)buf[i];
    }
    res = (char *)malloc(16);
    snprintf(res, 16, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
    return res;
}

int _build_port(char *buf)
{
    return (uint16_t)(buf[0] << 8 | buf[1]);
}


int _make_peers(peer **p, const char *buf, size_t buflen)
{
    if (buflen % 6 != 0)
    {
        printf("[peer] Poor format peers");
        return 0;
    }

    peer *head = NULL, *tail = NULL;
    char *pb = (char *)buf;
    
    for (size_t i = 0; i < buflen; i += 6)
    {
        char *ip = _build_ip(pb + i);
        int port = _build_port(pb + i + 4);
        peer *pt = (peer *)malloc(sizeof(peer));
        pt->ip = ip;
        pt->port = port;
        pt->next = NULL;
        
        if (head == NULL)
        {
            head = pt;
        }
        else
        {
            tail->next = pt;
        }
        tail = pt;
    }
    *p = head;
    printf("\n[peer] Find %lld peers!\n", buflen / 6);
    return 1;
}


int peer_init(peer **p, const char *buf, size_t buflen)
{
    char *peers;
    size_t peerslen;
    peerslen = _read_peers(&peers, buf, buflen);
    if (peerslen == 0)
    {
        return 0;
    }
    return _make_peers(p, peers, peerslen);
}

void peer_free(peer *p)
{
    while (p != NULL)
    {
        peer *pp = p;
        p = p->next;
        free(pp->ip);
        free(pp);
    }
}

char * _build_handshake(const char *info_hash, const char *peerid)
{
    char *hs_msg = (char *)malloc(sizeof(char) * 68);
    memset(hs_msg, 0, 68);
    memcpy(hs_msg, (char []){0x13}, 1);
    memcpy(hs_msg + 1, "BitTorrent protocol", 19);
    memcpy(hs_msg + 28, info_hash, 20);
    memcpy(hs_msg + 48, peerid, 20);
    return hs_msg;
}

int peer_handshake(peer *p, const char *info_hash, const char *peerid)
{
    int sock;
    char *handshake;
    char *recv, *recv_info_hash;
    if ((sock = tcp_connect(p->ip, p->port)) <= 0)
    {
        return 0;
    }
    
    handshake = _build_handshake(info_hash, peerid);
    tcp_send(sock, handshake, 68, &recv);
    recv_info_hash = (char *)malloc(sizeof(char) * 20);
    memcpy(recv_info_hash, recv + 28, 20);
    if (memcmp(recv_info_hash, info_hash, 20) != 0)
    {
        printf("[peer] Fail to handshake with %s:%d\n", p->ip, p->port);
        return 0;
    }
    return 1;
}