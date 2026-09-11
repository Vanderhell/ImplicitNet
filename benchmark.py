import time
from oracle import map_,unmap
for n in (16,64,256,1024,4096,65536):
    s=0x123456789abcde; q=100000
    t=time.perf_counter(); z=0
    for i in range(q): z^=map_(i&(n-1),n,s)
    dt=time.perf_counter()-t
    print(f'N={n:5d} map_ns={(dt/q)*1e9:.1f} sink={z}')
