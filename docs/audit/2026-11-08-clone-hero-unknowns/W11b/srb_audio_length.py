"""Compare a .srb bundle's stated song length with the length of its audio.

Usage: python -I srb_audio_length.py <bundle.srb> <scratch folder> <worktree root>

Reads the bundle as docs/srb-format.md lays it out: the 16-byte header, four
raw-DEFLATE streams, then the AES-128-CFB audio section. The stated length is
the 4-byte field in stream 1 that srb_parse_metadata reads. Each audio blob is
decrypted into the scratch folder (never into the repo), and its length is the
last Ogg page's granule position less the OpusHead pre-skip, at Opus's fixed
48 kHz granule rate. The AES key is read from src/app/preview_source.cpp, its
one home, rather than copied here.
"""
import os
import re
import struct
import sys
import zlib

from Crypto.Cipher import AES


def read_key(root):
    text = open(os.path.join(root, "src", "app", "preview_source.cpp"), encoding="utf-8").read()
    body = re.search(r"kSrbAesKey\[16\]\s*=\s*\{([^}]*)\}", text).group(1)
    return bytes(int(x, 16) for x in re.findall(r"0x([0-9a-fA-F]{2})", body))


def inflate(buf, offset):
    d = zlib.decompressobj(-15)
    out = d.decompress(buf[offset:])
    return out, len(buf) - len(d.unused_data)


def stated_length_ms(info):
    p = 4  # the fixed "4b4" + 1 prefix
    for _ in range(8):
        n = struct.unpack_from("<I", info, p)[0]
        p += 4 + n
    p += 12 + 4  # difficulty ratings, preview start
    n = struct.unpack_from("<I", info, p)[0]
    p += 4 + n  # icon name
    p += 8  # two track numbers
    return struct.unpack_from("<I", info, p)[0]


def opus_length_s(ogg):
    if ogg[:4] != b"OggS":
        return None, "not an Ogg stream"
    head = ogg.find(b"OpusHead")
    if head < 0:
        return None, "no OpusHead"
    pre_skip = struct.unpack_from("<H", ogg, head + 10)[0]
    last = ogg.rfind(b"OggS")
    granule = struct.unpack_from("<q", ogg, last + 6)[0]
    return (granule - pre_skip) / 48000.0, f"last granule {granule}, pre-skip {pre_skip}"


def main(path, scratch, root):
    key = read_key(root)
    buf = open(path, "rb").read()
    info, off = inflate(buf, 16)
    print(f"bundle: {os.path.basename(path)}, {len(buf)} bytes")
    ms = stated_length_ms(info)
    print(f"stated song length: {ms} ms = {ms / 1000:.3f} s")
    for _ in range(3):  # chart, two images
        _, off = inflate(buf, off)
    print(f"audio section starts at offset {off}")
    cursor = off
    first = True
    i = 0
    while cursor < len(buf):
        tag = struct.unpack_from("<i", buf, cursor)[0]
        cursor += 8
        header = buf[cursor:cursor + 16]
        size = struct.unpack_from("<Q", buf, cursor + 16)[0]
        cursor += 24
        enc = buf[cursor:cursor + size]
        cursor += size
        iv = header[8:16] + header[0:8]
        plain = AES.new(key, AES.MODE_CFB, iv=iv, segment_size=128).decrypt(enc)
        with open(os.path.join(scratch, f"blob{i}.opus"), "wb") as f:
            f.write(plain)
        secs, how = opus_length_s(plain)
        label = "song" if first else f"stem{i}"
        shown = f"{secs:.3f} s" if secs is not None else "no length"
        print(f"blob {i} ({label}): tag {tag}, {size} bytes, {shown} ({how})")
        first = False
        i += 1
    print(f"audio section ends at offset {cursor}; file size {len(buf)}")


if __name__ == "__main__":
    main(*sys.argv[1:4])
