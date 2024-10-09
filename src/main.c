#include <stdio.h>
#include "bencode.h"
#include "torrent.h"

int main() {
    torrent *tor = torrent_new();
    torrent_parse(tor, "test.torrent");
    torrent_hash *torh = torrent_hash_new();
    torrent_hash_hash(torh, tor);
    return 0;
}