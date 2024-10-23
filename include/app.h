#ifndef APP_H
#define APP_H

#define DEBUG

#include "torrent.h"
#include "http.h"
#include "peers.h"
#include "piecework.h"

typedef struct 
{
    torrent *tor;
    torrent_hash *torh;
    piecework *pw;
    char *peerid;
} app;

app * app_new(const char *filename);

int app_download(app *a, const char *dest);

void app_free(app *a);

#endif // APP_H