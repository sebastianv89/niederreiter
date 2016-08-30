#include <stdio.h>

int main(void) {
    union {
        char c;
        int i;
    } u;
    u.i = 0;
    u.c = 1;
    
    if (u.i & 1) {
        printf("LITTLE");
    } else {
        printf("BIG");
    }
    
    return 0;
}
