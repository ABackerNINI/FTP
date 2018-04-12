#include "Common.h"
#include <stdio.h>

int cut_string(char *_Str, const char _Cutter) {
    int n = 0;

    bool cutted = false;
    for (char *p = _Str; *p != '\0'; ++p) {
        if (*p == _Cutter) {
            *p = '\0';
            if (cutted) {
                ++n;
                cutted = false;
            }
        } else cutted = true;
    }
    if (cutted)++n;

    return n;
}

char upper(char c) {
    if (c >= 'a'&&c <= 'z')
        return c - 32;

    return c;
}

int stricmp_n_1(const char *_Str1, const char *_Str2) {
    for (const char *p = _Str1, *k = _Str2; *p != '\0'; ++p, ++k) {
        if (*p != upper(*k)) {
            return *p - upper(*k);
        }
    }
    return 0;
}
