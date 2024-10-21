#ifndef PEERS_H
#define PEERS_H

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "bencode.h"
#include "tcp.h"
#include "message.h"

struct _peer
{
    char *ip;
    int port;
    struct _peer *next;
};

typedef struct _peer peer;


int peer_init(peer **p, const char *buf, size_t buflen);


void peer_free(peer *p);


int peer_download(peer *p, const char *info_hash, const char *peerid);


#endif // PEERS_H