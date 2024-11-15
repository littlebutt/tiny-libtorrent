#include "torrent.h"

torrent *torrent_new()
{
    torrent *tor = (torrent *)calloc(sizeof(torrent), 1);
    return tor;
}

void torrent_free(torrent *tor)
{
    free(tor->announce);
    free(tor->info_pieces);
    free(tor->info_name);
    free(tor);
}

size_t _torrent_fsize(FILE *fp)
{
    fseek(fp, 0, SEEK_END);
    size_t fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    return fsize;
}

int _torrent_fopen(const char *filename, char **content)
{
    FILE *fp = NULL;
    size_t fsize = 0;
    char *buf = NULL;

    fp = fopen(filename, "rb");
    if (!fp)
    {
        return 0;
    }

    fsize = _torrent_fsize(fp);
    buf = (char *)calloc((fsize + 1) * sizeof(char), 1);
    fread(buf, fsize + 1, sizeof(char), fp);
    buf[fsize] = '\0';
    *content = (char *)calloc((fsize + 1) * sizeof(char), 1);
    memcpy(*content, buf, (fsize + 1) * sizeof(char));
    return fsize + 1;
}

int _torrent_decode(torrent **tor, const char *buf, size_t buflen)
{
    struct bencode ctx[1];
    int res = 0;
    // A flag for memoizing keys in bencode:
    // 2 for `announce`, 3 for `pieces`, 4 `piece length`, 5 for `length`, 6 for `name`
    int flag = 1;
    bencode_init(ctx, buf, buflen);
    // XXX: Assume all data we need is in a dict.
    while ((res = bencode_next(ctx)) > 0)
    {
        switch (res)
        {
        case BENCODE_STRING: {
            char *key = (char *)calloc((ctx->toklen + 1) * sizeof(char), 1);
            if (key == NULL)
            {
                return 0;
            }
            memcpy(key, ctx->tok, ctx->toklen);
            key[ctx->toklen] = '\0';
            switch (flag)
            {
            case 1: {
                if (strcmp(key, "announce") == 0)
                {
                    flag = 2;
                }
                else if (strcmp(key, "pieces") == 0)
                {
                    flag = 3;
                }
                else if (strcmp(key, "piece length") == 0)
                {
                    flag = 4;
                }
                else if (strcmp(key, "length") == 0)
                {
                    flag = 5;
                }
                else if (strcmp(key, "name") == 0)
                {
                    flag = 6;
                }
                else
                {
                    flag = 1;
                }
                break;
            }
            case 2: {
                (*tor)->announce = key;
                flag = 1;
                break;
            }
            case 3: {
                char *value = (char *)calloc(ctx->toklen * sizeof(char), 1);
                if (value == NULL)
                {
                    return 0;
                }
                memcpy(value, key, ctx->toklen * sizeof(char));
                (*tor)->info_pieces = value;
                (*tor)->_info_pieces_length = ctx->toklen;
                flag = 1;
                free(key);
                break;
            }
            case 6: {
                (*tor)->info_name = key;
                flag = 1;
                break;
            }
            case 4:
            case 5: {
                return 0;
            }
            }
            break;
        }
        case BENCODE_INTEGER: {
            char *key = (char *)calloc((ctx->toklen + 1) * sizeof(char), 1);
            memcpy(key, ctx->tok, ctx->toklen);
            key[ctx->toklen] = '\0';
            switch (flag)
            {
            case 4: {
                (*tor)->info_piece_length = (size_t)atoi(key);
                flag = 1;
                break;
            }
            case 5: {
                (*tor)->info_length = (size_t)atoi(key);
                flag = 1;
                break;
            }
            case 2:
            case 3:
            case 6: {
                return 0;
            }
            }
        }
        }
    }
    // FIXME: The ``res`` ends with -1 (BENCODE_ERROR_INVALID) but the data we
    // need is all parsed correctly.
    bencode_free(ctx);
    return res;
}

int torrent_parse(torrent *tor, const char *filename)
{
    char *buf;
    size_t buflen = 1;
    buflen = _torrent_fopen(filename, &buf);
    if (buflen <= 0)
    {
        return 0;
    }
    _torrent_decode(&tor, buf, buflen);
#ifdef DEBUG
    printf("torrent->announce: %s\n", tor->announce);
    printf("torrent->info_name: %s\n", tor->info_name);
    printf("torrent->info_piece_length: %zu\n", tor->info_piece_length);
    printf("torrent->info_length: %zu\n", tor->info_length);
    dbg_bin("torrent->info_pieces", tor->info_pieces, tor->_info_pieces_length);
#endif // DEBUG
    return 1;
}

torrent_hash *torrent_hash_new()
{
    torrent_hash *torh = (torrent_hash *)calloc(sizeof(torrent_hash), 1);
    return torh;
}

void torrent_hash_free(torrent_hash *torh)
{
    free(torh->info_hash);
    free(torh->pieces_hashes);
    free(torh);
}

size_t _torrent_hash_marshal_info(char **info, torrent *tor)
{
    char *pieces, *piece_length, *length, *name;
    char *_pieces_part0 = (char *)calloc(sizeof(char) * 20, 1);
    if (_pieces_part0 == NULL)
    {
        goto ERROR;
    }
    memset(_pieces_part0, 0, 20);
    sprintf(_pieces_part0, "6:pieces%zu:", tor->_info_pieces_length);
    pieces = (char *)calloc(sizeof(char) * ((tor->_info_pieces_length) + strlen(_pieces_part0)), 1);
    if (pieces == NULL)
    {
        goto ERROR;
    }
    memset(pieces, 0, (tor->_info_pieces_length) + strlen(_pieces_part0));
    if (pieces == NULL)
    {
        goto ERROR;
    }
    memcpy(pieces, _pieces_part0, strlen(_pieces_part0));
    memcpy(pieces + strlen(_pieces_part0), tor->info_pieces, tor->_info_pieces_length);
    piece_length = (char *)calloc(sizeof(char), 512);
    if (piece_length == NULL)
    {
        goto ERROR;
    }
    sprintf(piece_length, "12:piece lengthi%zue", tor->info_piece_length);
    length = (char *)calloc(sizeof(char), 512);
    if (length == NULL)
    {
        goto ERROR;
    }
    sprintf(length, "6:lengthi%zue", tor->info_length);
    name = (char *)calloc(sizeof(char), 2048);
    if (name == NULL)
    {
        goto ERROR;
    }
    sprintf(name, "4:name%zu:%s", strlen(tor->info_name), tor->info_name);
    size_t length_len = strlen(length);
    size_t name_len = strlen(name);
    size_t piece_length_len = strlen(piece_length);
    size_t pieces_len = (tor->_info_pieces_length) + strlen(_pieces_part0);
    size_t infolen = sizeof(char) * (2 + length_len + name_len + piece_length_len + pieces_len);
    *info = (char *)calloc(infolen, 1);
    memset(*info, 0, 2 + length_len + name_len + piece_length_len + pieces_len);
    memcpy(*info, "d", 1);
    memcpy(*info + 1, length, length_len);
    memcpy(*info + 1 + length_len, name, name_len);
    memcpy(*info + 1 + length_len + name_len, piece_length, piece_length_len);
    memcpy(*info + 1 + length_len + name_len + piece_length_len, pieces, pieces_len);
    memcpy(*info + 1 + length_len + name_len + piece_length_len + pieces_len, "e", 1);
    free(_pieces_part0);
    free(length);
    free(name);
    free(piece_length);
    free(pieces);
#ifdef DEBUG
    dbg_bin("marshaled info", *info, infolen);
#endif // DEBUG
    return infolen;
ERROR:
    return 0;
}

int _torrent_hash_hash_info(char *hashed_info, char *info, size_t infolen)
{
    SHA1_CTX ctx;
    SHA1Init(&ctx);
    SHA1Update(&ctx, (const unsigned char *)info, infolen);
    SHA1Final((unsigned char *)hashed_info, &ctx);
    return 1;
}

int torrent_hash_hash(torrent_hash *torh, torrent *tor)
{
    assert(tor);
    char *info = NULL;
    char *hashed_info, *pieces_hashes;
    size_t infolen = _torrent_hash_marshal_info(&info, tor);
    hashed_info = (char *)calloc(sizeof(char), 20);
    _torrent_hash_hash_info(hashed_info, info, infolen);
    torh->info_hash = hashed_info;
#ifdef DEBUG
    dbg_bin("hashed_info", hashed_info, 20);
#endif // DEBUG
    pieces_hashes = (char *)calloc(tor->info_piece_length, sizeof(char));
    memcpy(pieces_hashes, tor->info_pieces, tor->info_piece_length);
    torh->pieces_hashes = pieces_hashes;
    return 1;
}
