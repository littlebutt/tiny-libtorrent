#include <stdio.h> // debug
#include "bencode.h"

void bencode_init(bencode_t *be, char *str, size_t len) {
    be->str = be->start = str;
    be->len = len;
}

int bencode_is_int(bencode_t *be) {
    return be->str && *be->str == 'i';
}

int bencode_is_string(bencode_t *be) {
    char *sp = be->str;
    if (!isdigit(*sp)) {
        return 0;
    }
    do {
        sp ++;
    } while(isdigit(*sp));
    return *sp == ':';
}

int bencode_is_list(bencode_t *be) {
    return be->str && *be->str == 'l';
}

int bencode_is_dict(bencode_t *be) {
    return be->str && *be->str == 'd';
}

int bencode_dict_has_next(bencode_t *be) {
    char *sp = be->str;
    if (!sp ||
        // end of a string 
        *sp == '\0' || *sp == '\r' ||
        // end of a list or a dict
        *sp == 'e' ||
        // an empty dict or list
        (*sp == 'd' && *(sp + 1) == 'e') ||
        (*sp == 'l' && *(sp + 1) == 'e')) {
            return 0;
    }
    return 1;
}

char *_bencode_str_len(char *sp, size_t *strlen) {
    *strlen = 0;

    if (!isdigit(*sp)) {
        return NULL;
    }

    do {
        *strlen *= 10;
        *strlen += *sp - '0';
        sp ++;
    } while(isdigit(*sp));

    if (*sp != ':') {
        return NULL;
    }
    return sp + 1;
}

int _bencode_carry_length(bencode_t * be, char *pos) {
    return be->len - (pos - be->str);
}

char *_bencode_iterate_to_next_string_pos(bencode_t *be, char *sp) {
    bencode_t *iter = (bencode_t *)malloc(sizeof(bencode_t));
    bencode_init(iter, sp, _bencode_carry_length(be, sp));

    if (bencode_is_dict(iter)) {
        /* navigate to the end of the dictionary */
        while (bencode_dict_has_next(iter))
        {
            /* ERROR: input string is invalid */
            bencode_t *dummy = (bencode_t *)malloc(sizeof(bencode_t));
            if (0 == bencode_dict_get_next(iter, dummy)) {
                free(dummy);
                return NULL;
            }
            free(dummy);
        }

        /* special case for empty dict */
        if (*(iter->str) == 'd' && *(iter->str + 1) == 'e')
            return iter->str + 2;

        return iter->str + 1;
    } else if (bencode_is_string(iter)) {
        char *str;
        size_t *strlen = 0;

        /* ERROR: input string is invalid */
        if (0 == bencode_string_value(iter, &str, strlen))
            return NULL;

        return str + *strlen;
    }
    return NULL;
}

int bencode_dict_get_next(bencode_t *be, bencode_t *val) {
    char *sp = be->str;
    size_t strlen = 0;

    if (*sp == 'e') {
        return 0;
    }

    char *cur = _bencode_str_len(sp, &strlen);
    if (!cur) {
        return 0;
    };

    bencode_init(val, cur + strlen, _bencode_carry_length(be, cur + strlen));
    if (!(be->str = _bencode_iterate_to_next_string_pos(be, cur + strlen)))
    {
        return 0;
    }

    return 1;
}

int bencode_string_value(bencode_t *be, char **str, size_t *strlen) {
    char *sp = _bencode_str_len(be->str, strlen);
    
    /*  make sure we still fit within the buffer */
    if (sp + *strlen > be->start + (size_t) be->len)
    {
        *str = NULL;
        return 0;
    }

    *str = sp;
    return 1;

}



