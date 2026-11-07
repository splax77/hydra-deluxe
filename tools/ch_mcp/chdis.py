"""Disassemble Clone Hero's GameAssembly.dll by RVA, with method names.

This rewrites the lost capstone wrapper that made the `.asm` listings under
docs/audit/note-shuffle and docs/audit/drum-lanes, and prints them the same
way (tests/test_chdis.py pins one). It reads three things: the DLL's PE
section table (where each section sits in the file and in memory), the
method names Il2CppDumper wrote to `script.json` in a build's dump folder,
and the bytes at an RVA (an offset from the DLL's load base). `dis` prints a
listing in the committed listings' line format.

Use it two ways. `Image(dll_path, dump_dir)` opens one DLL and one dump
folder. The module-level names (`data`, `secs`, `BASE`, `names`, `meta`,
`rva2off`, `dis`) are the old helpers' interface: they open the installed
build on first use, with the game folder from game_files and the dump folder
from build_facts.

From a shell, at the repo root:
    py -m tools.ch_mcp.chdis <rva hex> [maxbytes hex] [--dll PATH] [--dumps DIR]
"""

from __future__ import annotations

import argparse
import json
import os
import re
import sys

if not __package__:
    # The old helpers under docs/audit import this file as a top-level
    # `chdis`; the repo root then has to be on the path for the imports below.
    sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))

from tools.ch_mcp import build_facts, dump_build, game_files
from tools.ch_probe.process import decode_double, decode_f32, decode_u32, decode_u64

SCRIPT_FILE = "script.json"
DEFAULT_MAXBYTES = 0x1200

# The listings put the name of a called method in a comment that starts at
# this column, so names line up down the page.
_COMMENT_COLUMN = 58
_MNEMONIC_WIDTH = 6

# An int3 right after one of these ends the listing: it is the padding after
# the method's last instruction. A jmp counts with any operand, the others
# only bare. An int3 after a call is the trap the compiler puts after a call
# that never returns, so the listing goes on; so does one after `ret 0`,
# as the committed listing open34-refute/ctor_6d60.asm shows.
_ENDS_BEFORE_PADDING = ("ret", "jmp", "int3")

# A RIP-relative memory operand as capstone writes it, with its size word.
_RIP_OPERAND = re.compile(
    r"(?:(?P<size>\w+) ptr )?\[rip (?P<sign>[+-]) (?P<disp>0x[0-9a-f]+)\]")
# The constants the listings decode, by operand size, in bytes.
_CONSTANT_SIZES = {"dword": 4, "qword": 8, "xmmword": 16}


class ChdisError(Exception):
    """The DLL or the dump folder cannot be read. The message names the path."""


def _u32(data: bytes, off: int) -> int:
    return decode_u32(data[off:off + 4])


def _u16(data: bytes, off: int) -> int:
    # process.py, the one home of byte decoding, has no 16-bit reader, so
    # this reads the 32 bits at `off` and keeps the low half. Every 16-bit
    # field read here has two more header bytes after it.
    return _u32(data, off) & 0xFFFF


def _read_sections(data: bytes, path: str) -> tuple[int, list[tuple[str, int, int, int, int]]]:
    """The image base and the section table of the PE file in `data`."""
    try:
        pe = _u32(data, 0x3C)
        if data[pe:pe + 4] != b"PE\0\0":
            raise ChdisError(f"{path} is not a PE file")
        count = _u16(data, pe + 6)
        opt_size = _u16(data, pe + 20)
        opt = pe + 24
        if _u16(data, opt) != 0x20B:
            raise ChdisError(f"{path} is not a 64-bit PE file")
        base = decode_u64(data[opt + 24:opt + 32])
        secs = []
        for i in range(count):
            row = opt + opt_size + 40 * i
            name = data[row:row + 8].rstrip(b"\0").decode("ascii", "replace")
            vsize, va, rsize, raw = (_u32(data, row + 8 + 4 * k) for k in range(4))
            secs.append((name, va, vsize, raw, rsize))
    except ValueError as e:
        raise ChdisError(f"{path} is cut short: {e}") from e
    return base, secs


def _read_names(dump_dir: str) -> tuple[dict[int, str], dict[int, str]]:
    """From script.json: the method names keyed by RVA, and the labels of
    the metadata, metadata-method and string-literal slots keyed by RVA, each
    written as the listings print it. Where several entries share an
    address, the last one in the file names it, as in the listings."""
    path = os.path.join(dump_dir, SCRIPT_FILE)
    if not os.path.isfile(path):
        raise ChdisError(f"no {SCRIPT_FILE} at {path}")
    try:
        with open(path, encoding="utf-8") as f:
            script = json.load(f)
        names = {m["Address"]: m["Name"] for m in script["ScriptMethod"]}
        meta: dict[int, str] = {}
        for m in script.get("ScriptMetadata", []):
            meta[m["Address"]] = f"meta:{m['Name']}"
        for m in script.get("ScriptMetadataMethod", []):
            meta[m["Address"]] = f"metamethod:{m['Name']}"
        for m in script.get("ScriptString", []):
            meta[m["Address"]] = f"str:{m['Value']!r}"
    except (OSError, ValueError, KeyError, TypeError) as e:
        raise ChdisError(f"cannot read method names from {path}: {type(e).__name__}: {e}") from e
    return names, meta


class Image:
    """One GameAssembly.dll and the dump folder that names its methods."""

    def __init__(self, dll_path: str, dump_dir: str):
        try:
            with open(dll_path, "rb") as f:
                self.data = f.read()
        except OSError as e:
            raise ChdisError(f"cannot read {dll_path}: {e}") from e
        self.BASE, self.secs = _read_sections(self.data, dll_path)
        self.names, self.meta = _read_names(dump_dir)
        self._disassembler = None

    def rva2off(self, rva: int) -> int:
        """The file offset of `rva`, from the section that holds it."""
        for name, va, vsize, raw, rsize in self.secs:
            if va <= rva < va + rsize:
                return raw + rva - va
        raise ChdisError(f"rva {rva:#x} is in no section's file bytes")

    def _cs(self):
        if self._disassembler is None:
            import capstone
            self._disassembler = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_64)
        return self._disassembler

    def lines(self, rva: int, maxbytes: int = DEFAULT_MAXBYTES) -> list[str]:
        """The listing of up to `maxbytes` bytes from `rva`, one line per
        instruction. It stops early at the padding after the method."""
        off = self.rva2off(rva)
        out = []
        previous = None
        for addr, size, mnemonic, ops in self._cs().disasm_lite(
                self.data[off:off + maxbytes], self.BASE + rva):
            text = f"{addr - self.BASE:08x}: {mnemonic:<{_MNEMONIC_WIDTH}} {ops}"
            comment = self._comment(addr - self.BASE + size, ops)
            if comment is not None:
                text = f"{text:<{_COMMENT_COLUMN}}; {comment}"
            out.append(text)
            if mnemonic == "int3" and previous in _ENDS_BEFORE_PADDING:
                break
            previous = mnemonic if mnemonic == "jmp" else f"{mnemonic} {ops}".strip()
        return out

    def _comment(self, next_rva: int, ops: str) -> str | None:
        """What an operand points at: the method an absolute address names,
        or for a RIP-relative operand (an address counted from the next
        instruction) its RVA and what lives there."""
        if ops.startswith("0x"):
            try:
                return self.names.get(int(ops, 16) - self.BASE)
            except ValueError:
                return None
        m = _RIP_OPERAND.search(ops)
        if m is None:
            return None
        disp = int(m.group("disp"), 16)
        target = next_rva + (disp if m.group("sign") == "+" else -disp)
        what = self._slot(target, m.group("size"))
        return f"[{target:#x}]" + (f" {what}" if what else "")

    def _slot(self, rva: int, size: str | None) -> str | None:
        """The label at `rva`, or "(bss)" when no file bytes back it, or the
        constant stored there for an operand of a size the listings decode."""
        if rva in self.meta:
            return self.meta[rva]
        try:
            off = self.rva2off(rva)
        except ChdisError:
            return "(bss)"
        if size not in _CONSTANT_SIZES:
            return None
        blob = self.data[off:off + _CONSTANT_SIZES[size]]
        if len(blob) != _CONSTANT_SIZES[size]:
            return None
        if size == "qword":
            return f"dbl={decode_double(blob)!r} q={decode_u64(blob):#x}"
        if size == "dword":
            return f"flt={decode_f32(blob)!r} i={decode_u32(blob)}"
        doubles = tuple(decode_double(blob[i:i + 8]) for i in (0, 8))
        floats = tuple(decode_f32(blob[i:i + 4]) for i in (0, 4, 8, 12))
        return f"xmm dbl={doubles!r} flt={floats!r}"

    def dis(self, rva: int, maxbytes: int = DEFAULT_MAXBYTES, out=None) -> None:
        """Print the listing of `lines` to `out` (standard output by default)."""
        out = sys.stdout if out is None else out
        for line in self.lines(rva, maxbytes):
            print(line, file=out)


def installed_paths() -> tuple[str, str]:
    """The installed game's DLL and its build's dump folder."""
    try:
        dll, _metadata = dump_build.game_inputs(game_files.GAME_DIR)
        build = build_facts.build_number(build_facts.current_build())
    except (dump_build.DumpError, build_facts.BuildFactsError) as e:
        raise ChdisError(f"cannot find the installed build: {e}") from e
    return dll, os.path.join(build_facts.DUMPS_ROOT, build)


_installed: Image | None = None
_LEGACY = ("data", "secs", "BASE", "names", "meta", "rva2off", "dis")


def __getattr__(attr: str):
    """The old helpers' module-level names, from the installed build."""
    global _installed
    if attr not in _LEGACY:
        raise AttributeError(f"module {__name__!r} has no attribute {attr!r}")
    if _installed is None:
        _installed = Image(*installed_paths())
    return getattr(_installed, attr)


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Disassemble GameAssembly.dll at an RVA.")
    parser.add_argument("rva", help="the start, in hex")
    parser.add_argument("maxbytes", nargs="?", default=f"{DEFAULT_MAXBYTES:x}",
                        help="the most bytes to read, in hex")
    parser.add_argument("--dll", help="GameAssembly.dll (default: the installed game's)")
    parser.add_argument("--dumps", help="the build's dump folder holding script.json "
                        "(default: the installed build's)")
    args = parser.parse_args(argv)
    sys.stdout.reconfigure(encoding="utf-8")
    try:
        dll, dumps = installed_paths() if not (args.dll and args.dumps) else (args.dll, args.dumps)
        image = Image(args.dll or dll, args.dumps or dumps)
        image.dis(int(args.rva, 16), int(args.maxbytes, 16))
    except ChdisError as e:
        print(f"chdis: {e}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
