#include "app.h"

app * app_new(const char *filename)
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
    return a;
}

void app_free(app *a)
{
    torrent_free(a->tor);
    torrent_hash_free(a->torh);
    peer_free(a->p);
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

    char *encoded_info_hash = http_url_encode(a->torh->info_hash, 20);

    sprintf(url, "%s?compact=1&downloaded=0&info_hash=%s&left=%lld&peer_id=%s&port=6881&uploaded=0", a->tor->announce, encoded_info_hash, a->tor->info_length, a->peerid);
    return url;
}

int app_download(app *a, const char *dest)
{
    printf("%s", a->torh->info_hash);
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
    while (p)
    {
        peer_download(p, a->torh->info_hash, a->peerid);
        p = p->next;
    }
    
    return 0;
  }