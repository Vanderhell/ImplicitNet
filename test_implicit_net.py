import ctypes
import os
import random
from collections import deque
from oracle import bits, neighbor, next_, slow_xor_next

library_path = os.environ.get('IMPLICIT_NET_LIBRARY', './implicit_net.dll')
DLL = ctypes.CDLL(library_path); U32=ctypes.c_uint32; U64=ctypes.c_uint64
DLL.in_next.argtypes=[U32,U32,U32,U64]; DLL.in_next.restype=U32
DLL.in_neighbor.argtypes=[U32,U32,U32,U64]; DLL.in_neighbor.restype=U32
def active_bits(n): return [0] if bits(n)==2 else list(range(4*bits(n)-8))
def neighbors(x,n,s): return [neighbor(x,d,n,s) for d in range(bits(n))]
def score(y,target,kind):
    z=y^target
    if kind=='hamming': return (z.bit_count(),z)
    if kind=='xor': return (z,z.bit_count())
    return (z.bit_length(),z.bit_count(),z)
def local_next(x,target,n,s,kind): return min(neighbors(x,n,s),key=lambda y:score(y,target,kind))
def shortest(source,target,n,s):
    q=deque([(source,0)]); seen={source}
    while q:
        x,d=q.popleft()
        if x==target:return d
        for y in neighbors(x,n,s):
            if y not in seen:seen.add(y);q.append((y,d+1))
    raise AssertionError('disconnected')
def route(source,target,n,s,kind):
    x=source; visited={x}; hops=0
    for _ in range(4*n):
        if x==target:return True,hops,0,False
        y=next_(x,target,n,s) if kind=='structural' else local_next(x,target,n,s,kind)
        x=y; hops+=1
        if x in visited:return False,hops,0,True
        visited.add(x)
    return False,hops,0,True
def cube_neighbors(x,n): return [x^(1<<d) for d in range(bits(n))]
def cube_route(source,target,n,kind):
    x=source; seen={x}
    for hops in range(1,4*n+1):
        y=min(cube_neighbors(x,n),key=lambda q:score(q,target,kind))
        if y in seen:return False,hops,True
        if y==target:return True,hops,False
        seen.add(y);x=y
    return False,4*n,True
def cases(n,rng):
    w=bits(n); states=32 if n<256 else 16; pairs=64; out=[]
    for _ in range(states):
        s=rng.getrandbits(1 if w==2 else 4*w-8)
        for _ in range(pairs):
            a=rng.randrange(n);b=rng.randrange(n)
            if a!=b:out.append((a,b,s))
    return out
def collect(n,work,kind):
    rows=[]
    for a,b,s in work:
        dist=shortest(a,b,n,s); ok,hops,sw,loop=route(a,b,n,s,kind)
        rows.append((ok,hops,sw,loop,dist))
    success=[r for r in rows if r[0]]
    return {'total':len(rows),'success':len(success),'loops':sum(r[3] for r in rows),
            'avg_hops':sum(r[1] for r in success)/len(success) if success else 0,
            'worst_hops':max(r[1] for r in rows),'avg_shortest':sum(r[4] for r in rows)/len(rows),
            'avg_stretch':sum(r[1]/r[4] for r in success)/len(success) if success else 0,
            'worst_stretch':max(r[1]/r[4] for r in success) if success else 0,
            'avg_switches':sum(r[2] for r in rows)/len(rows)}
def cube_stats(n,work,kind):
    rows=[]
    for a,b,_ in work:
        ok,hops,loop=cube_route(a,b,n,kind);dist=(a^b).bit_count();rows.append((ok,hops,loop,dist))
    return sum(r[0] for r in rows),len(rows),sum(r[2] for r in rows),sum(r[1] for r in rows)/len(rows)
def run():
    rng=random.Random(0xA701); checks=0
    for _ in range(100000):
        n=1<<rng.choice([2,3,4,5,6,7,8,9,10,12,16]); s=rng.getrandbits(64);x=rng.randrange(n);t=rng.randrange(n);d=rng.randrange(bits(n))
        assert DLL.in_neighbor(x,d,n,s)==neighbor(x,d,n,s)
        assert DLL.in_next(x,t,n,s)==next_(x,t,n,s)==slow_xor_next(x,t,n,s); checks+=1
    print('C_NEXT_ORACLE_PASS vectors=%d' % checks)
    for n in (32,64,256):
        work=cases(n,rng)
        data={k:collect(n,work,k) for k in ('hamming','xor','hybrid','structural')}
        best=min(data,key=lambda k:(-data[k]['success'],data[k]['avg_stretch'],data[k]['loops']))
        cube=cube_stats(n,work,'xor')
        assert data['structural']==data['xor']
        print('ROUTING N=%d heuristics=%s best=%s cube=(success=%d/%d loops=%d avg_hops=%.4f)' % (n,data,best,*cube))
if __name__=='__main__':run()
