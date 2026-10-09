"""Independent byte-level read of a .mid's PART DRUMS track: print note-on pitches per tick in
file order (tom markers shown as tomY/tomB/tomG). usage: py -I mid_order.py <notes.mid>"""
import struct, sys

data = open(sys.argv[1], "rb").read()
fmt, ntrk, div = struct.unpack(">HHH", data[8:14])
pos = 14
P = {96: "kick", 97: "red", 98: "Y", 99: "B", 100: "G", 110: "tomY", 111: "tomB", 112: "tomG"}
print("division", div)
for _ in range(ntrk):
    assert data[pos:pos + 4] == b"MTrk"
    ln = struct.unpack(">I", data[pos + 4:pos + 8])[0]
    p, end, t, name, ons = pos + 8, pos + 8 + ln, 0, None, {}
    while p < end:
        d = 0
        while True:
            b = data[p]; p += 1
            d = (d << 7) | (b & 0x7F)
            if not b & 0x80:
                break
        t += d
        st = data[p]
        if st == 0xFF:
            typ, l = data[p + 1], data[p + 2]
            if typ == 3:
                name = data[p + 3:p + 3 + l].decode()
            p += 3 + l
        else:
            if st & 0xF0 == 0x90 and data[p + 2] > 0:
                ons.setdefault(t, []).append(P.get(data[p + 1], data[p + 1]))
            p += 3
    if name == "PART DRUMS":
        for t in sorted(ons):
            print(t, ons[t])
    pos = end
