#include "utils.h"

void dbg_bin(const char *label, const char *bin, size_t binlen)
{
    printf("%s: ", label);
    for (size_t i = 0; i < binlen; ++i)
    {
        printf("\\x%02x", (unsigned char)bin[i]);
    }
    printf("\n");
}