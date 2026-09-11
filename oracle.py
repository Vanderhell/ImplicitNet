MASK64=(1<<64)-1
def valid(n): return 4<=n<=65536 and n&(n-1)==0
def bits(n): return n.bit_length()-1
def mix(x):
    x^=x>>30; x=(x*0xbf58476d1ce4e5b9)&MASK64; x^=x>>27
    x=(x*0x94d049bb133111eb)&MASK64; return (x^(x>>31))&MASK64
def map_(x,n,s):
    if not valid(n): return None
    w=bits(n); z=mix(s); y=0
    for i in range(w):
        p=(i+((z>>4)&15))%w
        if (z>>16)&1: p=w-1-p
        if (x>>i)&1: y|=1<<p
    return (y^(z&0xffffffff))&(n-1)
def unmap(y,n,s):
    if not valid(n): return None
    w=bits(n); z=mix(s); y=(y^(z&0xffffffff))&(n-1); x=0
    for i in range(w):
        p=(i+((z>>4)&15))%w
        if (z>>16)&1: p=w-1-p
        if (y>>p)&1: x|=1<<i
    return x
def direct_mode(s,d): return (s>>(2*d-2))&3
def alternating_mode(s,d,w): return 0 if d>=w-1 else (s>>(2*w+2*d-6))&3
def first_frame(s): return 1 if (s&3)==2 else (2 if (s&3)==3 else 0)
def first_twist(s,block):
    c=s&3
    if c==0:return 0
    if c==1:return block&1
    if c==2:return 1^(block&1)
    return 1
def oriented_mode(s,d,block=0,w=None):
    if d==1:return first_twist(s,block)
    m=first_frame(s)
    for i in range(2,d+1): m^=direct_mode(s,i)
    if block&1:m^=alternating_mode(s,d,w)
    return m
def twist_mask(d,m):
    if m==0:return 0
    if m==1:return 1
    if m==2:return 1<<(d-1)
    return (1<<d)-1
def neighbor(x,d,n,s):
    if not valid(n) or x>=n or d>=bits(n): return None
    if d==0:return x^1
    if bits(n)==2 and d==1:return x^2^(1 if s&1 else 0)
    return x^(1<<d)^twist_mask(d,oriented_mode(s,d,x>>(d+1),bits(n)))
def next_(current,target,n,s):
    if not valid(n) or current>=n or target>=n:return None
    delta=current^target
    return neighbor(current,0,n,s) if delta==0 else neighbor(current,delta.bit_length()-1,n,s)
def slow_xor_next(current,target,n,s):
    return min((neighbor(current,d,n,s) for d in range(bits(n))),key=lambda y: (y^target,(y^target).bit_count()))
def step(s,e,n): return (s^mix(e^(n<<32)^0x9e3779b97f4a7c15))&MASK64
def prev(s,e,n): return step(s,e,n)
