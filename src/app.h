#ifndef APP_H
#define APP_H

#include "torrent.h"
#include "http.h"

typedef struct 
{
    torrent *tor;
    torrent_hash *torh;
} app;

app * app_new(const char *filename);

int app_download(app *a, const char *dest);

void app_free(app *a);

#endif // APP_H