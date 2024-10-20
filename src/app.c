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

    sprintf(url, "%s?compact=1&downloaded=0&info_hash=%s&left=%lld&peer_id=-TR2940-k8hj0wgej6ch&port=6881&uploaded=0", a->tor->announce, encoded_info_hash, a->tor->info_length);
    return url;
}

int app_download(app *a, const char *dest)
{
    char *url = _build_url(a);
    char *recv, *body;
    size_t content_length;
    http_get(url, &recv);
    content_length = (size_t)atoi(http_response_header(recv, "Content-Length"));
    body = http_response_body(recv, content_length);
    printf("%s", body);
    return 0;
  }