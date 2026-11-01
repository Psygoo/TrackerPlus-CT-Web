#include <stddef.h>
void *memset(void *p,int value,size_t n) {
    volatile unsigned char *d=p;
    while(n--)*d++=(unsigned char)value;
    return p;
}
