
#include "string.h"

int string_equals(const char *a, const char *b) {
    if (a == b) {
        return 1;
    }
    while (1) {
        char ca = *a;
        char cb = *b;
        if (ca != cb) {
            return 0;
        } else if (ca == 0) {
            return 1;
        } else {
            a++;
            b++;
        }
    }
}
