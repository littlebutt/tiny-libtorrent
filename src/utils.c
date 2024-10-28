#include "utils.h"

void dbg_bin(const char *label, const char *bin, size_t binlen)
{
    size_t len;
#ifdef DEBUG
    len = binlen;
#else
    len = 20 > binlen ? binlen : 20;
#endif
    printf("%s: ", label);
    for (size_t i = 0; i < len; ++i)
    {
        printf("\\x%02x", (unsigned char)bin[i]);
    }
#ifndef DEBUG
    if (binlen > 20)
    {
        printf("...(%d more)", binlen - 20);
    }
#endif
    printf("\n");
}