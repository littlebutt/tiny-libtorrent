#pragma once

#include "bencode.h"
#include "sha1.h"
#include "utils.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char *announce;
    char *info_pieces;
    size_t _info_pieces_length; // Real size of `pieces`
    size_t info_piece_length;
    size_t info_length;
    char *info_name;

} torrent;

typedef struct {
    char *info_hash;
    char *pieces_hashes;
} torrent_hash;

torrent *torrent_new();

void torrent_free(torrent *tor);

int torrent_parse(torrent *tor, const char *filename);

torrent_hash *torrent_hash_new();

void torrent_hash_free(torrent_hash *torh);

int torrent_hash_hash(torrent_hash *torh, torrent *tor);
