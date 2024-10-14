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

char * _build_url(app *a)
{
    char *url = (char *)malloc(sizeof(char) * 1024);
    if (url == NULL)
    {
        return NULL;
    }
    memset(url, 0, 1024);

    char *encoded_info_hash = http_url_encode(a->torh->info_hash);
    char *peer_id = "tinylibtorrent_cli";
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
    // http://bttracker.debian.org:6969/announce?compact=1&downloaded=0&info_hash=%1B%D0%88%EE%91f%A0b%CFJ%F0%9C%F9%97+%FAn%1A13&left=661651456&peer_id=%7D%F2%D2%5D%08s%22%BC%60E7%00S%AD%D1%F6%B0%91%BE%07&port=6881&uploaded=0
    // http://bttracker.debian.org:6969/announce?compact=1&downloaded=0&info_hash=%0fU%82~tu%cb%fb%bbp2%afX%ef%ce%27N%14%93a%fb%7f&left=661651456&peer_id=tinylibtorrent_cli&port=6881&uploaded=0
}