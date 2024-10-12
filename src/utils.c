#include "utils.h"

void dbg_bin(char *bin, size_t binlen)
{
    for (size_t i=0; i<binlen; ++i)
    {
        printf("%d", bin[i]);
    }
}