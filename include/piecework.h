#ifndef PIECEWORK_H
#define PIECEWORK_H

#include "torrent.h"

struct _piecework
{
    int index;
    char *hash;
    size_t length;
    struct _piecework *next;
};

typedef struct _piecework piecework;

int piecework_build(piecework **pw, const torrent *tor);

void piecework_free(piecework *pw);

piecework * piecework_append(piecework *pw, piecework *node, piecework *prev);

int piecework_has_piece(char *bitfield, int bitfieldlen, int index);

int piecework_set_piece(char *bitfield, int bitfieldlen, int index);

#endif