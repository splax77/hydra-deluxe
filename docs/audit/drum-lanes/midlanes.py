import sys, struct

def vlq(d, i):
    v = 0
    while True:
        b = d[i]; i += 1
        v = (v << 7) | (b & 0x7F)
        if not b & 0x80:
            return v, i

def tracks(path):
    d = open(path, 'rb').read()
    res = struct.unpack('>H', d[12:14])[0]
    i = 14
    while i < len(d):
        cid, ln = d[i:i+4], struct.unpack('>I', d[i+4:i+8])[0]
        yield res, d[i+8:i+8+ln]
        i += 8 + ln

def events(t):
    i, tick, run, name = 0, 0, 0, ''
    out = []
    while i < len(t):
        dt, i = vlq(t, i); tick += dt
        s = t[i]
        if s == 0xFF:
            typ = t[i+1]; ln, j = vlq(t, i+2)
            if typ == 3: name = t[j:j+ln].decode('latin1')
            i = j + ln; continue
        if s in (0xF0, 0xF7):
            ln, j = vlq(t, i+1); i = j + ln; continue
        if s & 0x80: run = s; i += 1
        hi = run & 0xF0
        if hi in (0xC0, 0xD0): i += 1; continue
        a, b = t[i], t[i+1]; i += 2
        if hi in (0x80, 0x90): out.append((tick, hi == 0x90 and b > 0, a, b))
    return name, out

path = sys.argv[1] if __name__ == '__main__' else None
for res, t in (tracks(path) if path else []):
    name, ev = events(t)
    if name != 'PART DRUMS': continue
    print('res', res)
    for tick, on, p, v in ev:
        if p in (126, 127): print('lane', tick, 'on' if on else 'off', p, 'vel', v)
    first = [e for e in ev if e[1] and e[2] in (126, 127)][:1]
    if first:
        t0 = first[0][0]
        print('notes after first lane start:', [(tk, p) for tk, on, p, v in ev if on and 95 <= p <= 100 and t0 <= tk < t0 + 400])
