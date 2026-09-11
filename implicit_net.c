#include "implicit_net.h"

static int valid_n(uint32_t n) { return n >= 4u && n <= 65536u && (n & (n - 1u)) == 0u; }
static unsigned width(uint32_t n) {
    unsigned w=0u;
    if(n==65536u)return 16u;
    if(n>=256u){n>>=8;w+=8u;}
    if(n>=16u){n>>=4;w+=4u;}
    if(n>=4u){n>>=2;w+=2u;}
    if(n>=2u)++w;
    return w;
}
static uint32_t nmask(uint32_t n) { return n==65536u ? UINT32_C(65535) : n-1u; }
static uint64_t mix64(uint64_t x) {
    x^=x>>30; x*=UINT64_C(0xbf58476d1ce4e5b9); x^=x>>27;
    x*=UINT64_C(0x94d049bb133111eb); return x^(x>>31);
}

/* Mapping layer: a bijection, deliberately independent of neighbor(). */
static uint32_t map_core(uint32_t x,uint32_t n,in_state s) {
    unsigned w=width(n),i; uint32_t y=0u,m=nmask(n); uint64_t z=mix64(s);
    for(i=0u;i<w;++i) {
        unsigned p=(unsigned)((i+((z>>4)&15u))%w);
        if(((z>>16u)&1u)!=0u) p=w-1u-p;
        if(((x>>i)&1u)!=0u) y|=UINT32_C(1)<<p;
    }
    return (y^(uint32_t)z)&m;
}
static uint32_t unmap_core(uint32_t y,uint32_t n,in_state s) {
    unsigned w=width(n),i; uint32_t x=0u,m=nmask(n); uint64_t z=mix64(s);
    y=(y^(uint32_t)z)&m;
    for(i=0u;i<w;++i) {
        unsigned p=(unsigned)((i+((z>>4)&15u))%w);
        if(((z>>16u)&1u)!=0u) p=w-1u-p;
        if(((y>>p)&1u)!=0u) x|=UINT32_C(1)<<i;
    }
    return x;
}
int in_init(in_state *s,uint32_t n,uint64_t seed) { if(s==0||!valid_n(n))return 0;*s=seed;return 1; }
uint32_t in_map(uint32_t x,uint32_t n,in_state s) { return valid_n(n)?map_core(x,n,s):UINT32_MAX; }
uint32_t in_unmap(uint32_t y,uint32_t n,in_state s) { return valid_n(n)?unmap_core(y,n,s):UINT32_MAX; }

/* Compact layout for N=2^w, w>=3:
   c1: bits 0..1; m_i (i=2..w-1): bits 2i-2..2i-1;
   a_i (i=2..w-2): bits 2w+2i-6..2w+2i-5. */
static unsigned alternating_mode(in_state s,unsigned dimension,unsigned width_bits) {
    if(dimension>=width_bits-1u)return 0u;
    return (unsigned)((s >> (2u * width_bits + 2u * dimension - 6u)) & 3u);
}
/* Prefix XOR is a small, state-derived orientation frame.  A lower
   dimension changes the interpretation of every later dimension. */
static unsigned first_frame(in_state s) {
    unsigned c=(unsigned)(s&3u);
    return c==2u ? 1u : (c==3u ? 2u : 0u);
}
static unsigned first_twist(in_state s,uint32_t block) {
    unsigned c=(unsigned)(s&3u);
    if(c==0u)return 0u;
    if(c==1u)return (unsigned)(block&1u);
    if(c==2u)return 1u^(unsigned)(block&1u);
    return 1u;
}
static unsigned parity64(uint64_t x) {
    x^=x>>32; x^=x>>16; x^=x>>8; x^=x>>4; x^=x>>2; x^=x>>1;
    return (unsigned)(x&1u);
}
static unsigned oriented_mode(in_state s,unsigned dimension,uint32_t block,unsigned width_bits) {
    uint64_t fields; unsigned mode;
    if(dimension==1u)return first_twist(s,block);
    mode=first_frame(s);
    fields=s&((UINT64_C(1)<<(2u*dimension))-UINT64_C(4));
    mode^=parity64(fields&UINT64_C(0x5555555555555554));
    mode^=parity64(fields&UINT64_C(0xaaaaaaaaaaaaaaa8))<<1;
    if((block&1u)!=0u) mode^=alternating_mode(s,dimension,width_bits);
    return mode;
}
static uint32_t twist_mask(unsigned dimension,unsigned mode) {
    uint32_t low=(UINT32_C(1)<<dimension)-1u;
    if(mode==0u)return 0u;                 /* straight */
    if(mode==1u)return 1u;                 /* exchange low coordinate */
    if(mode==2u)return UINT32_C(1)<<(dimension-1u); /* half twist */
    return low;                            /* local reversal i -> low-i */
}
static unsigned highest_bit(uint32_t x) {
    unsigned d=0u;
    if(x>=UINT32_C(256)){x>>=8;d+=8u;}
    if(x>=UINT32_C(16)){x>>=4;d+=4u;}
    if(x>=UINT32_C(4)){x>>=2;d+=2u;}
    if(x>=UINT32_C(2))++d;
    return d;
}
uint32_t in_neighbor(uint32_t x,uint32_t direction,uint32_t n,in_state s) {
    unsigned w;
    if(!valid_n(n)||x>=n)return UINT32_MAX;
    w=width(n);
    if(direction>=w)return UINT32_MAX;
    if(direction==0u)return x^1u;
    if(w==2u && direction==1u) return x^(UINT32_C(1)<<1)^((s&1u)!=0u ? 1u : 0u);
    return x^(UINT32_C(1)<<direction)^twist_mask(direction,oriented_mode(s,direction,x>>(direction+1u),w));
}
uint32_t in_next(uint32_t current,uint32_t target,uint32_t n,in_state s) {
    uint32_t delta;
    if(!valid_n(n)||current>=n||target>=n)return UINT32_MAX;
    delta=current^target;
    if(delta==0u)return in_neighbor(current,0u,n,s);
    return in_neighbor(current,highest_bit(delta),n,s);
}
int in_step(in_state *s,uint32_t n,uint64_t event) {
    if(s==0||!valid_n(n))return 0;
    *s^=mix64(event^((uint64_t)n<<32)^UINT64_C(0x9e3779b97f4a7c15)); return 1;
}
int in_prev(in_state *s,uint32_t n,uint64_t event) { return in_step(s,n,event); }
