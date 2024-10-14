#include <stdio.h>
#include "app.h"

int main() {
    // torrent *tor = torrent_new();
    // torrent_parse(tor, "test.torrent");
    // torrent_hash *torh = torrent_hash_new();
    // torrent_hash_hash(torh, tor);

    // gcc -g -Wall -O0 bencode.c torrent.c sha1.c main.c utils.c http.c -lws2_32
    // char * recv = NULL;
    // http_get("cn.bing.com/search?q=strcmp&qs=n&form=QBRE&sp=-1&lq=0&pq=strcmp&sc=10-6&sk=&cvid=B7543C43B3F54F9291EC92B24ACF2294&ghsh=0&ghacc=0&ghpl=", &recv);
    // printf("%s", recv);

    app *a = app_new("test.torrent");
    app_download(a, NULL);

    return 0;
}