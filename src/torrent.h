#ifndef TORRENT_H
#define TORRENT_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "bencode.h"
#include "sha1.h"

typedef struct
{
    char *announce;
    char *info_pieces;
    size_t info_piece_length;
    size_t info_length;
    char *info_name;

} torrent;

typedef struct
{
    char *info_hash;
    char **pieces_hashes;
} torrent_hash;

torrent * torrent_new();

void torrent_free(torrent *tor);

int torrent_parse(torrent *tor, const char *filename);

torrent_hash * torrent_hash_new();

void torrent_hash_free(torrent_hash *torh);

int torrent_hash_hash(torrent_hash *torh, torrent *tor);

#endif // TORRENT_H
