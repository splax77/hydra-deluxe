"""Tests for chdis.py.

Most tests run on a tiny PE file built by hand in a temp folder, with a
script.json beside it, so they never touch the game. One test reads the real
GameAssembly.dll and the 6142 dump when this machine has both, and checks the
listing against a committed one.
"""

from __future__ import annotations

import json
import os
import struct

import pytest

from tools.ch_mcp import build_facts, chdis, game_files

TINY_BASE = 0x140000000

# The code at rva 0x1000 of the tiny PE, one instruction per row.
TINY_CODE = bytes.fromhex(
    "53"                # push rbx
    "e81a000000"        # call the method at rva 0x1020
    "8b05f40f0000"      # a dword at rva 0x2000
    "488d0ded110000"    # rva 0x2200, past .data's file bytes
    "488b15ee0f0000"    # rva 0x2008, a string literal's slot
    "5b"                # pop rbx
    "c3"                # ret
    "cccc"              # padding
    "cccc"
)


def _tiny_pe() -> bytes:
    """A 64-bit PE file with a .text section at rva 0x1000 (file 0x400) and a
    .data section at rva 0x2000 (file 0x600) whose memory runs past its
    0x200 file bytes."""
    out = bytearray(0x800)
    out[0:2] = b"MZ"
    struct.pack_into("<I", out, 0x3C, 0x40)
    out[0x40:0x44] = b"PE\0\0"
    opt_size = 0xF0
    struct.pack_into("<HH", out, 0x44, 0x8664, 2)
    struct.pack_into("<H", out, 0x54, opt_size)
    opt = 0x58
    struct.pack_into("<H", out, opt, 0x20B)
    struct.pack_into("<Q", out, opt + 24, TINY_BASE)
    table = opt + opt_size
    for i, (name, va, vsize, raw, rsize) in enumerate((
            (b".text", 0x1000, 0x200, 0x400, 0x200),
            (b".data", 0x2000, 0x400, 0x600, 0x200))):
        row = table + 40 * i
        out[row:row + 8] = name.ljust(8, b"\0")
        struct.pack_into("<IIII", out, row + 8, vsize, va, rsize, raw)
    out[0x400:0x400 + len(TINY_CODE)] = TINY_CODE
    out[0x420] = 0xC3
    struct.pack_into("<f", out, 0x600, 100.0)
    return bytes(out)


def _script(tmp_path) -> str:
    dumps = tmp_path / "dumps"
    dumps.mkdir()
    (dumps / chdis.SCRIPT_FILE).write_text(json.dumps({
        "ScriptMethod": [{"Address": 0x1000, "Name": "Demo$$Start"},
                         {"Address": 0x1020, "Name": "Demo$$Target"}],
        "ScriptString": [{"Address": 0x2008, "Value": "hi"}],
        "ScriptMetadata": [],
        "ScriptMetadataMethod": [],
    }), encoding="utf-8")
    return str(dumps)


@pytest.fixture
def tiny(tmp_path) -> chdis.Image:
    dll = tmp_path / "GameAssembly.dll"
    dll.write_bytes(_tiny_pe())
    return chdis.Image(str(dll), _script(tmp_path))


def test_rva2off_reads_each_section_of_the_table(tiny):
    assert tiny.BASE == TINY_BASE
    assert [s[0] for s in tiny.secs] == [".text", ".data"]
    assert tiny.rva2off(0x1000) == 0x400
    assert tiny.rva2off(0x1004) == 0x404
    assert tiny.rva2off(0x2010) == 0x610
    assert tiny.rva2off(0x21FF) == 0x7FF


@pytest.mark.parametrize("rva", [0x500, 0x1200, 0x2200, 0x3000])
def test_rva2off_refuses_an_rva_with_no_file_bytes(tiny, rva):
    with pytest.raises(chdis.ChdisError, match=f"rva {rva:#x} is in no section"):
        tiny.rva2off(rva)


def test_dis_prints_the_listing_up_to_the_padding(tiny, capsys):
    tiny.dis(0x1000, 0x40)
    assert capsys.readouterr().out.splitlines() == [
        "00001000: push   rbx",
        "00001001: call   0x140001020                              ; Demo$$Target",
        "00001006: mov    eax, dword ptr [rip + 0xff4]             ; [0x2000] flt=100.0 i=1120403456",
        "0000100c: lea    rcx, [rip + 0x11ed]                      ; [0x2200] (bss)",
        "00001013: mov    rdx, qword ptr [rip + 0xfee]             ; [0x2008] str:'hi'",
        "0000101a: pop    rbx",
        "0000101b: ret    ",
        "0000101c: int3   ",
    ]


def test_dis_stops_at_maxbytes(tiny):
    assert tiny.lines(0x1000, 6) == [
        "00001000: push   rbx",
        "00001001: call   0x140001020                              ; Demo$$Target",
    ]


def test_a_missing_script_json_is_reported_by_its_path(tmp_path):
    dll = tmp_path / "GameAssembly.dll"
    dll.write_bytes(_tiny_pe())
    empty = tmp_path / "no-dump"
    empty.mkdir()
    with pytest.raises(chdis.ChdisError) as err:
        chdis.Image(str(dll), str(empty))
    assert str(err.value) == f"no script.json at {os.path.join(str(empty), 'script.json')}"


REAL_DLL = os.path.join(game_files.GAME_DIR, "GameAssembly.dll")
REAL_DUMPS = os.path.join(build_facts.DUMPS_ROOT, "6142")
REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))
PIN = os.path.join(REPO, "docs", "audit", "drum-lanes", "play", "f20deec0.asm")


def test_real_dll_matches_the_committed_listing():
    for path in (REAL_DLL, os.path.join(REAL_DUMPS, chdis.SCRIPT_FILE)):
        if not os.path.isfile(path):
            pytest.skip(f"no {path} on this machine")
    with open(PIN, encoding="utf-8") as f:
        pin_first = f.readline().rstrip("\n")
    lines = chdis.Image(REAL_DLL, REAL_DUMPS).lines(0x20DEEC0, 64)
    assert lines[0] == pin_first
