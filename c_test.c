#include "implicit_net.h"
#include <stdio.h>
#include <time.h>

static volatile uint32_t sink;
static uint64_t checks;

static int check_state(uint32_t n, in_state s) {
    uint32_t x; unsigned d,w=0u;
    for(x=n;x>1u;x>>=1)++w;
    for (x = 0u; x < n; ++x) {
        uint32_t y = in_map(x, n, s);
        in_state t;
        if (y >= n || in_unmap(y, n, s) != x || in_map(in_unmap(x, n, s), n, s) != x) return 10;
        for(d=0u;d<w;++d) {
            uint32_t m=in_neighbor(x,d,n,s),e;
            if(m>=n||m==x||in_neighbor(m,d,n,s)!=x)return 11;
            for(e=0u;e<d;++e)if(m==in_neighbor(x,e,n,s))return 12;
        }
        t = s; if (!in_step(&t, n, UINT64_C(0x13579bdf)) || !in_prev(&t, n, UINT64_C(0x13579bdf)) || t != s) return 14;
        ++checks;
    }
    return 0;
}
static double bench_ns(uint64_t it, uint32_t n, in_state s, unsigned op) {
    clock_t start = clock(); uint64_t i;
    for (i = 0u; i < it; ++i) {
        uint32_t x = (uint32_t)i & (n - 1u);
        if (op == 0u) sink ^= in_map(x, n, s);
        else if (op == 1u) sink ^= in_unmap(x, n, s);
        else if (op == 2u) sink ^= in_neighbor(x, (unsigned)i % (n == 4u ? 2u : 8u), n, s);
        else if (op == 3u) sink ^= in_neighbor(x, 0u, n, s);
        else if (op == 4u) { in_state t = s; in_step(&t, n, i); sink ^= (uint32_t)t; }
        else if (op == 5u) { in_state t = s; in_prev(&t, n, i); sink ^= (uint32_t)t; }
        else sink ^= in_next(x, (uint32_t)(i * UINT64_C(0x9e3779b9)) & (n - 1u), n, s);
    }
    return 1e9 * (double)(clock() - start) / (double)CLOCKS_PER_SEC / (double)it;
}
int main(void) {
    const uint32_t sizes[] = {4u,8u,16u,32u,64u,128u,256u,512u,1024u,65536u};
    const in_state fixed[] = {0u,1u,UINT64_MAX,UINT64_C(0xaaaaaaaaaaaaaaaa),UINT64_C(0x5555555555555555),UINT64_C(0x123456789abcdef0)};
    size_t i, j;
    for (i = 0u; i < sizeof(sizes)/sizeof(sizes[0]); ++i) {
        for (j = 0u; j < sizeof(fixed)/sizeof(fixed[0]); ++j) if (check_state(sizes[i], fixed[j]) != 0) return 2;
        for (j = 0u; j < 64u; ++j) if (check_state(sizes[i], UINT64_C(1) << j) != 0 || check_state(sizes[i], ~(UINT64_C(1) << j)) != 0) return 3;
    }
    if (in_init(0, 4u, 0u) || in_init(&(in_state){0u}, 0u, 0u) || in_init(&(in_state){0u}, 2u, 0u) || in_init(&(in_state){0u}, 3u, 0u)) return 4;
    if (in_neighbor(0u, 2u, 4u, 0u) != UINT32_MAX) return 5;
    printf("PASS C invariants checks=%llu\n", (unsigned long long)checks);
    for (i = 0u; i < 2u; ++i) { uint32_t n = i == 0u ? 4u : 65536u; const uint64_t it = 2000000u;
        printf("C_O2_NS N=%u map=%.2f unmap=%.2f neighbor=%.2f neighbor0=%.2f step=%.2f prev=%.2f next=%.2f sink=%u\n", n,
          bench_ns(it,n,fixed[5],0u),bench_ns(it,n,fixed[5],1u),bench_ns(it,n,fixed[5],2u),bench_ns(it,n,fixed[5],3u),bench_ns(it,n,fixed[5],4u),bench_ns(it,n,fixed[5],5u),bench_ns(it,n,fixed[5],6u),sink); }
    return 0;
}
