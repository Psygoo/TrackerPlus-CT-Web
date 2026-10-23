#include <stdint.h>
typedef uint32_t (*value_fn)(uint32_t);
typedef uint32_t (*rng_fn)(int32_t,int32_t);
typedef void (*text_fn)(void *,char *,uint32_t,uint32_t,uint32_t,uint32_t);

uint32_t h12_random_value(int32_t lo,int32_t hi,uint32_t fixed_type)
{
    uint32_t value=((rng_fn)0x3AF5u)(lo,hi);
    if (fixed_type==4u) {
        if (value>141u) value=141u;
        return ((value_fn)0x601BC4A9u)(value);
    }
    return value;
}

void h12_fill_text(void *seq,char *out,uint32_t size,uint32_t type,
                   uint32_t rank,uint32_t compact)
{
    if (type!=4u || rank<=100u || rank>141u) {
        ((text_fn)0x600AFB49u)(seq,out,size,type,rank,compact);
        return;
    }
    uint32_t value=((value_fn)0x601BC4A9u)(rank);
    char label[4];
    const char *text=label;
    if (value>=138u) {
        text=(const char *)0x601BC274u+((value-138u)>>1)*4u;
        if ((value&1u)==0u) ++text;
    } else if (value==101u) {
        label[0]='1';label[1]='S';label[2]=0;
    } else if (value==102u) {
        label[0]='!';label[1]='1';label[2]='S';label[3]=0;
    } else {
        uint32_t ab=((const uint8_t *)0x601BAEFCu)[value-103u];
        label[0]=(char)('1'+(ab>>4));label[1]=':';
        label[2]=(char)('0'+(ab&15u));label[3]=0;
    }
    if (size) {
        uint32_t i=0;
        while (i+1u<size && text[i]) {out[i]=text[i];++i;}
        out[i]=0;
    }
}
