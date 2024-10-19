#include "app.h"

app * app_new(const char *filename)
{
    app *a = (app *)malloc(sizeof(app));
    a->tor = torrent_new();
    torrent_parse(a->tor, filename);
    a->torh = torrent_hash_new();
    torrent_hash_hash(a->torh, a->tor);
    return a;
}

void app_free(app *a)
{
    torrent_free(a->tor);
    torrent_hash_free(a->torh);
    free(a);
}

void _generate_peer_id(char *peer_id)
{
    char *p = peer_id;
    for (int i = 0; i < 20; ++i)
    {
        *p = (unsigned char)(rand() % 256);
        ++ p;
    }
}

char * _build_url(app *a)
{
    char *url = (char *)malloc(sizeof(char) * 1024);
    if (url == NULL)
    {
        return NULL;
    }
    memset(url, 0, 1024);

    char *encoded_info_hash = http_url_encode(a->torh->info_hash);
    char peer_id[20] = {0};
    _generate_peer_id(peer_id);
    char *encoded_peer_id = http_url_encode(peer_id);

    sprintf(url, "%s?compact=1&downloaded=0&info_hash=%s&left=%lld&peer_id=%s&port=6881&uploaded=0", a->tor->announce, encoded_info_hash, a->tor->info_length, encoded_peer_id);
    return url;
}

int app_download(app *a, const char *dest)
{
    char *url = _build_url(a);
    char *recv;
    http_get(url, &recv);
    printf("recv: %s", recv);
    return 0;
  }