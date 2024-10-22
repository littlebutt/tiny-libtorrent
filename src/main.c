#include "app.h"

int main() {
    app *a = app_new("..\\test.torrent"); // TODO: custom args
    app_download(a, NULL);

    return 0;
}