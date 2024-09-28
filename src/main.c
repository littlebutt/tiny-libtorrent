#include <stdio.h>
#include "bencode.h"
#include "torrent.h"

int main() {
    torrent *tor = NULL;
    torrent_new(&tor);
    int res = torrent_parse(tor, "test.torrent");
    return res;
}