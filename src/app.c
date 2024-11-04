#include "app.h"

app *app_new(const char *filename)
{
    app *a = (app *)malloc(sizeof(app));
    a->tor = torrent_new();
    if (torrent_parse(a->tor, filename) == 0)
    {
        return 0;
    }
    a->torh = torrent_hash_new();
    torrent_hash_hash(a->torh, a->tor);
    a->peerid = "-TR2940-k8hj0wgej6ch";
    piecework_build(&a->pw, a->tor);
    return a;
}

void app_free(app *a)
{
    torrent_free(a->tor);
    torrent_hash_free(a->torh);
    piecework_free(a->pw);
    free(a);
}

char *_build_url(app *a)
{
    char *url = (char *)malloc(sizeof(char) * 1024);
    if (url == NULL)
    {
        return NULL;
    }
    memset(url, 0, 1024);

    char *encoded_info_hash = http_url_encode(a->torh->info_hash, 20);

    sprintf(url, "%s?compact=1&downloaded=0&info_hash=%s&left=%zu&peer_id=%s&port=6881&uploaded=0",
            a->tor->announce, encoded_info_hash, a->tor->info_length, a->peerid);
    return url;
}

int _integrate_peer_result(char **buf, peer_result *res, const torrent *tor)
{
    peer_result *pr = res;
    *buf = (char *)malloc(tor->info_length);
    memset(*buf, 0, tor->info_length);
    if (buf == NULL)
    {
        return 0;
    }
    for (int i = 0; i < tor->_info_pieces_length / 20; i++)
    {
        int begin = pr->index * tor->info_piece_length;
        int end = begin + tor->info_piece_length > tor->info_length
                      ? tor->info_length
                      : begin + tor->info_piece_length;
        memcpy(*buf + begin, res->buf, end - begin);

        printf("[app] Downloaded piece #%d - %0.2f%%", res->index,
               (float)i / (tor->_info_pieces_length / 20));
    }
    return 1;
}

int app_download(app *a, const char *dest)
{
    char *url = _build_url(a);
    char *recv, *body;
    size_t content_length;
    peer *ph = NULL;
    http_get(url, &recv);
#ifdef DEBUG
    printf("%s", recv);
#endif
    content_length = (size_t)atoi(http_response_header(recv, "Content-Length"));
    if (content_length == 0)
    {
        return 0;
    }
    body = http_response_body(recv, content_length);
    peer_init(&ph, body, content_length);
    peer *p = ph;
    peer_result *result = NULL;
    while (p)
    {
        peer_result *res = peer_download(p, a->torh->info_hash, a->peerid, a->pw,
                                         a->tor->_info_pieces_length / 20);
        if (result == NULL)
        {
            result = res;
        }
        else
        {
            peer_result *pr = result;
            while (pr->next != NULL)
            {
                pr = pr->next;
            }
            pr->next = res;
        }
        p = p->next;
    }
    char *buf = NULL;
    if (!_integrate_peer_result(&buf, result, a->tor))
    {
        printf("[app] Fail to integrate the result buffer\n");
        return 0;
    }

    const char *path = dest == NULL ? a->tor->info_name : dest;

    FILE *fp = fopen(path, "wb");
    if (fp == NULL)
    {
        printf("[app] Fail to create path %s\n", path);
        return 0;
    }
    fwrite(buf, a->tor->info_length, 1, fp);
    fclose(fp);

    return 1;
}