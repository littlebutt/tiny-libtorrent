#include "app.h"

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("Too less variants!\nUsages: tiny-libtorrent torrent_filename");
        exit(1);
    }

    app *a = app_new(argv[1]);
    app_download(a, NULL);
    app_free(a);

    return 0;
}