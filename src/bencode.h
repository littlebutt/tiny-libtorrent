#ifndef BENCODE_H
#define BENCODE_H

#include <stdlib.h>
#include <ctype.h>

typedef struct {
    char *str;      // The bencode string expected to read
    size_t len;    // The length of `str`
    char *start;    // The start char of `str`
} bencode_t;

void bencode_init(bencode_t *be, char *str, size_t len);


int bencode_is_int(bencode_t *be);

int bencode_is_string(bencode_t *be);

int bencode_is_list(bencode_t *be);

int bencode_is_dict(bencode_t *be);

int bencode_dict_has_next(bencode_t *be);

int bencode_dict_get_next(bencode_t *be, bencode_t *val);

int bencode_string_value(bencode_t *be, char **str, size_t *strlen);

#endif // BENCODE_H