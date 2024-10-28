#include "piecework.h"


int _calculate_length(int index, const size_t piece_length, const size_t length)
{
    size_t begin = index * piece_length;
    size_t end = begin + piece_length > length ? length : begin + piece_length;
    return (int)(end - begin);
}


int piecework_build(piecework **pw, const torrent *tor)
{
    piecework *head = NULL;
    piecework *ppw = NULL;
    piecework *pre = NULL;
    char *ph = tor->info_pieces;
    size_t hashes_size = tor->_info_pieces_length / 20;
    for (size_t i = 0; i < hashes_size; ++i)
    {
        ppw = (piecework *)malloc(sizeof(piecework));
        if (ppw == NULL)
        {
            // FIXME: memory leak
            return 0;
        }
        ppw->index = (int)i;
        ppw->hash = ph;
        ppw->length = _calculate_length(i, tor->info_piece_length, tor->info_length);
        ppw->next = NULL;
        if (head == NULL)
        {
            head = ppw;
        }
        else
        {
            pre->next = ppw;
        }
        pre = ppw;
        ph += 20;
    }
    *pw = head;
    return 1;
}

void piecework_free(piecework *pw)
{
    while (pw != NULL)
    {
        piecework *pp = pw;
        pw = pw->next;
        free(pp->hash);
        free(pp);
    }
}

piecework * piecework_append(piecework *pw, piecework *node, piecework *prev)
{
    if (pw == NULL || node == NULL || node->next == NULL)
    {
        return pw;
    }

    if (pw == node)
    {
        pw = node->next;
    }
    else if (prev != NULL)
    {
        prev->next = node->next;
    }

    piecework *last = pw;
    while (last->next != NULL) {
        last = last->next;
    }

    last->next = node;
    node->next = NULL;

    return pw;
}

int piecework_has_piece(char *bitfield, int bitfieldlen, int index)
{
    int byteIndex = index / 8;
	int offset = index % 8;
	if (byteIndex < 0 || byteIndex >= bitfieldlen)
    {
		return 0;
	}
	return bitfield[byteIndex] >> ((uint8_t)(7 - offset) & 1) != 0;
}

int piecework_set_piece(char *bitfield, int bitfieldlen, int index)
{
    int byteIndex = index / 8;
	int offset = index % 8;
	if (byteIndex < 0 || byteIndex >= bitfieldlen)
    {
		return 0;
	}
    bitfield[byteIndex] |= 1 << (int)(7 - offset);
    return 1;
}