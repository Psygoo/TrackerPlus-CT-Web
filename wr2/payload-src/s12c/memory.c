#include <stddef.h>
void *memset(void *p,int value,size_t n) {
    volatile unsigned char *d=p;
    while(n--)*d++=(unsigned char)value;
    return p;
}

void *memcpy(void *p,const void *q,size_t n) {volatile unsigned char *d=p;const unsigned char *s=q;while(n--)*d++=*s++;return p;}
