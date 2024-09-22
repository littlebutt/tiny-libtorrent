#include <stdio.h>
#include "bencode.h"

int main() {
    bencode_t *be = (bencode_t *)malloc(sizeof(bencode_t));
    bencode_init(be, "d3:foo3:bare", 12);
    if (bencode_is_dict(be)) {
        if (bencode_dict_has_next(be)) {
            bencode_t *val = (bencode_t *)malloc(sizeof(bencode_t));
            int res = bencode_dict_get_next(be, val);
            printf("res is : %d", res);
            printf("dict value is %s", val->str);
        }
    } else if (bencode_is_string(be)) {
        char *val;
        size_t strlen;
        bencode_string_value(be, &val, &strlen);
        printf("string value is: %s\n", val);
        printf("string length is %lld\n", strlen);
    }
    return 0;
}