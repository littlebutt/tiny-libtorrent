#ifndef PEERS_H
#define PEERS_H

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "bencode.h"
#include "tcp.h"
#include "message.h"
#include "piecework.h"

struct _peer
{
    char *ip;
    int port;
    struct _peer *next;
};

typedef struct _peer peer;

typedef struct
{
    int index;
    char *buf;
    size_t buflen;

    int downloaded;
    int requested;
    int backlog;
}_peer_state;


typedef struct
{
    int sock;
    int choked;
    char *bitfield;
    int bitfieldlen;
    char *info_hash;
    _peer_state *state;
}_peer_context;


int peer_init(peer **p, const char *buf, size_t buflen);


void peer_free(peer *p);


int peer_download(peer *p, char *info_hash, const char *peerid, piecework* pw);

#endif // PEERS_H