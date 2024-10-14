#ifndef PEERS_H
#define PEERS_H
struct _peer
{
    char *ip;
    int port;
    struct _peer *next;
};

typedef struct _peer peer;


int make_peers(peer *p, const char *data);


#endif // PEERS_H