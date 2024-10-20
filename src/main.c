#include <stdio.h>
#include "app.h"

int main() {
    // torrent *tor = torrent_new();
    // torrent_parse(tor, "test.torrent");
    // torrent_hash *torh = torrent_hash_new();
    // torrent_hash_hash(torh, tor);

    // gcc -g -Wall -O0 bencode.c torrent.c sha1.c main.c utils.c http.c -lws2_32
    // char * recv = NULL;

    app *a = app_new("test.torrent");
    app_download(a, NULL);

    return 0;
}