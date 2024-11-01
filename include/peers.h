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
#include "sha1.h"

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
}_peer_state;


typedef struct
{
    int sock;
    int choked;
    char *bitfield;
    int bitfieldlen;
    char *info_hash;
    _peer_state *state;
    int io_flag;
}_peer_context;


typedef struct _peer_result
{
    int index;
    char *buf;
    size_t buflen;
    struct _peer_result *next;
}peer_result;


int peer_init(peer **p, const char *buf, size_t buflen);


void peer_free(peer *p);


peer_result * peer_download(peer *p, char *info_hash, const char *peerid, piecework* pw, int pwlen);

#endif // PEERS_H