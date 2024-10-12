#include "torrent.h"
#define DEBUG

torrent * torrent_new()
{
    torrent *tor = (torrent *)malloc(sizeof(torrent));
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
    if (!fp) {
        return 0;
    }

    fsize = _torrent_fsize(fp);
    buf = (char *)malloc((fsize + 1) * sizeof(char));
    fread(buf, fsize + 1, sizeof(char), fp);
    buf[fsize] = '\0';
    *content = (char *)malloc((fsize + 1) * sizeof(char));
    strncpy(*content, buf, (fsize + 1) * sizeof(char));
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
            case BENCODE_STRING:
            {
                char *key = (char *)malloc((ctx->toklen + 1) * sizeof(char));
                if (key == NULL)
                {
                    return 0;
                }
                memcpy(key, ctx->tok, ctx->toklen);
                key[ctx->toklen] = '\0';
                switch (flag) {
                    case 1: {
                        if (strcmp(key, "announce") == 0) {
                            flag = 2;
                        } else if (strcmp(key, "pieces") == 0) {
                            flag = 3;
                        } else if (strcmp(key, "piece length") == 0) {
                            flag = 4;
                        } else if (strcmp(key, "length") == 0) {
                            flag = 5;
                        } else if (strcmp(key, "name") == 0) {
                            flag = 6;
                        } else {
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
                        char *value = (char *)malloc(ctx->toklen * sizeof(char));
                        if (value == NULL) {
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
            case BENCODE_INTEGER:
            {
                char *key = (char *)malloc((ctx->toklen + 1) * sizeof(char));
                memcpy(key, ctx->tok, ctx->toklen);
                key[ctx->toklen] = '\0';
                switch (flag) {
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
    printf("torrent->info_piece_length: %lld\n", tor->info_piece_length);
    printf("torrent->info_length: %lld\n", tor->info_length);
    printf("torrent->info_pieces: %s\n", tor->info_pieces);
#endif
    return 1;
}

torrent_hash * torrent_hash_new()
{
    torrent_hash *torh = (torrent_hash *)malloc(sizeof(torrent_hash));
    return torh;
}

void torrent_hash_free(torrent_hash *torh)
{
	free(torh->info_hash);
	free(torh->pieces_hashes);
	free(torh);
}

int _torrent_hash_marshal_info(char **info, torrent *tor)
{
    char *pieces, *piece_length, *length, *name;
    // XXX
    pieces = (char *)malloc(sizeof(char) * (tor->info_piece_length * 2));
    if (pieces == NULL)
    {
        goto ERROR;
    }
    sprintf(pieces, "6:pieces%lld:%s", tor->_info_pieces_length, tor->info_pieces);
    piece_length = (char *)malloc(sizeof(char) * 512);
    if (piece_length == NULL)
    {
        goto ERROR;
    }
    sprintf(piece_length, "12:piece lengthi%llde", tor->info_piece_length);
    length = (char *)malloc(sizeof(char) * 512);
    if (length == NULL)
    {
        goto ERROR;
    }
    sprintf(length, "6:lengthi%llde", tor->info_length);
    name = (char *)malloc(sizeof(char) * 2048);
    if (name == NULL)
    {
        goto ERROR;
    }
    sprintf(name, "4:name%lld:%s", strlen(tor->info_name), tor->info_name);
    sprintf(*info, "d%s%s%s%se", length, name, piece_length, pieces);
#ifdef DEBUG
    printf("marshaled info:%s\n", *info);
#endif
    return 1;
ERROR:
    return 0;
}

int _torrent_hash_hash_info(char *hashed_info, char *info)
{
    SHA1_CTX ctx;
    SHA1Init(&ctx);
    size_t info_len = strlen(info);
    SHA1Update(&ctx, (const unsigned char*)info, info_len);
    SHA1Final((unsigned char *)hashed_info, &ctx);
    return 1;
}

int torrent_hash_hash(torrent_hash *torh, torrent *tor)
{
    assert(tor);
    char *info = NULL;
    char hashed_info[20];
    char *pieces_hashes;
    info = (char *)malloc(sizeof(char) * tor->info_length * 2);
    _torrent_hash_marshal_info(&info, tor); 
    _torrent_hash_hash_info(hashed_info, info);
    torh->info_hash = hashed_info;

    pieces_hashes = (char *)malloc(tor->info_piece_length * sizeof(char));
    strncpy(pieces_hashes, tor->info_pieces, tor->info_piece_length);
    torh->pieces_hashes = pieces_hashes;
    return 1;
}
