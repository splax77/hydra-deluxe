"""Tests for tools/ch_mcp/launch_patch.json, the one-byte patch the launch
agent makes in memory, checked against the rest of the repo: the facts table
and the loader's listing that task D1 of the direct-start plan wrote
(docs/superpowers/plans/2026-10-31-ch-mcp-direct-start.md (removed; git show 6804353f:docs/superpowers/plans/2026-10-31-ch-mcp-direct-start.md), task D5),
and one real human launch the launch agent recorded.

No expected value is typed here. Each one is read from the file that owns it:
the facts table, the listing, or the recorded run. The recording is copied
from hydra-data\\ch-mcp-runs\\2026-11-07\\ls-f1-run1 (LS-F1, Get Ur Freak On,
ProDrums Expert, human=True): its launch log is the launch agent's replies,
holding the game's own bytes at the loader and the patch site before the write
and after it; its scorestats file is what the game saved for that song; and
replies-md5.txt is the two lines of that run's notes that give the md5 of the
GameAssembly.dll it ran. This test never opens GameAssembly.dll or starts the
game.
"""

from __future__ import annotations

import csv
import json
import os
import re
import sys
import unittest

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_mcp import bot_run  # noqa: E402

PATCH_FILE = os.path.join(_REPO_ROOT, "tools", "ch_mcp", "launch_patch.json")
FACTS_FILE = os.path.join(_REPO_ROOT, "docs", "clone-hero", "code", "facts-6142.tsv")
LISTING_FILE = os.path.join(_REPO_ROOT, "docs", "clone-hero", "code", "6142",
                            "cli_loader_3968D0.asm")
RUN_DIR = os.path.join(_REPO_ROOT, "testdata", "ch_runs", "2026-11-07-ls-f1-run1")
RUN_LAUNCH_LOG = os.path.join(RUN_DIR, "launch-20261107-233501.jsonl")
RUN_SCORESTATS = os.path.join(
    RUN_DIR, "scorestats-Eric_Moore_-_Get_Ur_Freak_On_Missy_Elliot-20261107-233822.json")
RUN_MD5_NOTES = os.path.join(RUN_DIR, "replies-md5.txt")

# x86-64's register numbers, in the order the instruction encoding counts them.
_REGS64 = ["rax", "rcx", "rdx", "rbx", "rsp", "rbp", "rsi", "rdi",
           "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"]
_REGS32 = ["eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi"]


def _load_patch() -> dict:
    with open(PATCH_FILE, encoding="utf-8") as f:
        return json.load(f)


def _listing() -> list[tuple[int, str]]:
    """The listing's lines as (rva, instruction text with its spacing and
    comment dropped)."""
    out = []
    with open(LISTING_FILE, encoding="utf-8") as f:
        for line in f:
            address, _, rest = line.partition(":")
            text = " ".join(rest.split(";", 1)[0].split())
            out.append((int(address, 16), text))
    return out


def _listing_text_at(rva: int) -> str:
    texts = [text for address, text in _listing() if address == rva]
    if len(texts) != 1:
        raise AssertionError(f"the listing has {len(texts)} lines at {rva:#x}")
    return texts[0]


def _facts_rows() -> tuple[list[str], list[dict]]:
    """The facts table's comment lines and its rows."""
    with open(FACTS_FILE, encoding="utf-8", newline="") as f:
        lines = f.read().splitlines()
    comments = [line for line in lines if line.startswith("#")]
    body = [line for line in lines if not line.startswith("#")]
    return comments, list(csv.DictReader(body, delimiter="\t"))


def _facts_dll() -> tuple[str, str]:
    """The DLL the facts table was written from, and its md5, from the
    table's `# <dll> md5: <hex>` header line."""
    comments, _ = _facts_rows()
    found = [m for m in (re.fullmatch(r"# (\S+) md5: ([0-9a-fA-F]{32})", c) for c in comments)
             if m]
    if len(found) != 1:
        raise AssertionError(f"the facts table has {len(found)} md5 lines")
    return found[0].group(1), found[0].group(2).lower()


def _recorded_patched_status() -> dict:
    """The launch agent's first recorded reply that says the patch took."""
    with open(RUN_LAUNCH_LOG, encoding="utf-8") as f:
        statuses = [json.loads(line)["payload"] for line in f if line.strip()]
    patched = [s for s in statuses if s.get("state") == bot_run.STATE_PATCHED]
    if not patched:
        raise AssertionError(f"{RUN_LAUNCH_LOG} has no {bot_run.STATE_PATCHED} reply")
    return patched[0]


def _recorded_md5s() -> list[str]:
    with open(RUN_MD5_NOTES, encoding="utf-8") as f:
        return [m.lower() for m in re.findall(r"md5 \w+ ([0-9A-Fa-f]{32})", f.read())]


def _imm32(data: bytes) -> int:
    return int.from_bytes(data, "little", signed=True)


def _decode(code: bytes) -> str:
    """The instruction text of `code`, in the listing's words, for the three
    forms the patch's bytes hold: push of a 64-bit register (an optional REX
    prefix, then 0x50 + the register's low three bits), sub of an imm32 from a
    64-bit register (REX.W, 0x81, a ModRM byte whose reg field is 5), and mov of
    an imm32 into a 32-bit register (0xB8 + the register)."""
    rex = code[0] if 0x40 <= code[0] <= 0x4F else None
    body = code[1:] if rex is not None else code
    rex_b = 8 if rex is not None and rex & 1 else 0
    op = body[0]
    if 0x50 <= op <= 0x57 and len(body) == 1:
        return f"push {_REGS64[rex_b + (op - 0x50)]}"
    if op == 0x81 and rex is not None and rex & 8 and len(body) == 6:
        modrm = body[1]
        if modrm >> 6 == 3 and (modrm >> 3) & 7 == 5:
            return f"sub {_REGS64[rex_b + (modrm & 7)]}, {_imm32(body[2:]):#x}"
    if 0xB8 <= op <= 0xBF and rex is None and len(body) == 5:
        return f"mov {_REGS32[op - 0xB8]}, {_imm32(body[1:])}"
    raise AssertionError(f"no decoding for {code.hex(' ')}")


class LaunchPatchFileTest(unittest.TestCase):
    def setUp(self):
        self.patch = _load_patch()

    def test_it_patches_the_dll_the_facts_table_was_written_from(self):
        dll, _ = _facts_dll()
        self.assertEqual(self.patch["module"], dll)

    def test_launch_and_defer_hold_values_the_launch_knows(self):
        # The live steps change these two between runs, so only their kind is
        # pinned.
        self.assertIn(self.patch["launch"], bot_run.LAUNCH_FORMS)
        self.assertIsInstance(self.patch["defer"], bool)

    def test_expect_encodes_the_listings_instruction_and_is_what_the_game_held(self):
        rva = int(self.patch["patch_rva"], 16)
        self.assertEqual(_decode(bytes.fromhex(self.patch["expect"])), _listing_text_at(rva))
        self.assertEqual(self.patch["expect"], _recorded_patched_status()["before"])

    def test_write_made_the_game_build_a_human_player(self):
        # The recorded run wrote these bytes, the game read them back, and the
        # song it then played was saved as not a bot's: the written immediate
        # is the game's "is_bot" value for that player.
        status = _recorded_patched_status()
        self.assertEqual(status["after"], self.patch["write"])
        with open(RUN_SCORESTATS, encoding="utf-8") as f:
            players = json.load(f)["players"]
        self.assertEqual(len(players), 1, players)
        self.assertIs(players[0]["is_bot"], False)
        write = bytes.fromhex(self.patch["write"])
        self.assertEqual(_imm32(write[1:]), int(players[0]["is_bot"]))

    def test_write_differs_from_expect_in_the_one_byte_at_plus_one(self):
        expect = bytes.fromhex(self.patch["expect"])
        write = bytes.fromhex(self.patch["write"])
        self.assertEqual(len(write), len(expect))
        differ = [i for i in range(len(expect)) if expect[i] != write[i]]
        self.assertEqual(differ, [1])


class AgainstTheRepoTest(unittest.TestCase):
    def setUp(self):
        self.patch = _load_patch()

    def test_the_bytes_were_read_from_the_dll_the_facts_table_names(self):
        # The recorded run's game held the bytes this file expects, and its
        # DLL's md5 is the one the facts table was written from. A table
        # regenerated from another DLL no longer matches that run.
        status = _recorded_patched_status()
        self.assertEqual(status["entry_before"], self.patch["entry_expect"])
        self.assertEqual(status["before"], self.patch["expect"])
        _, table_md5 = _facts_dll()
        md5s = _recorded_md5s()
        self.assertTrue(md5s, RUN_MD5_NOTES)
        self.assertEqual(set(md5s), {table_md5})

    def test_the_build_is_the_one_the_table_and_the_recorded_run_name(self):
        comments, _ = _facts_rows()
        versions = [c.split(":", 1)[1].strip() for c in comments if c.startswith("# version:")]
        self.assertEqual(len(versions), 1, comments)
        with open(RUN_SCORESTATS, encoding="utf-8") as f:
            game_version = json.load(f)["game_version"]
        self.assertEqual(game_version.lstrip("v"), versions[0])
        self.assertTrue(versions[0].startswith(self.patch["build"] + "-"), versions[0])

    def test_the_loader_rva_has_a_method_row_in_the_facts_table(self):
        _, rows = _facts_rows()
        methods = [r for r in rows
                   if r["kind"] == "method" and int(r["rva"], 16) == int(self.patch["loader_rva"], 16)]
        self.assertEqual(len(methods), 1, methods)

    def test_the_listing_is_the_loaders_and_holds_the_patch_rva(self):
        listing = _listing()
        self.assertEqual(listing[0][0], int(self.patch["loader_rva"], 16))
        patch_rva = int(self.patch["patch_rva"], 16)
        self.assertEqual([a for a, _ in listing].count(patch_rva), 1,
                         f"the loader's listing has no single line at {patch_rva:#x}")

    def test_the_listing_opens_with_the_instructions_entry_expect_encodes(self):
        # Each listing line's address minus the one before it is that
        # instruction's length, so the listing cuts entry_expect into its
        # instructions; each piece must decode to its line's text.
        entry = bytes.fromhex(self.patch["entry_expect"])
        listing = _listing()
        start = listing[0][0]
        decoded = []
        for (address, text), (next_address, _) in zip(listing, listing[1:]):
            if next_address - start > len(entry):
                break
            piece = entry[address - start:next_address - start]
            decoded.append((_decode(piece), text))
        ends = [a - start for a, _ in listing]
        self.assertIn(len(entry), ends, "entry_expect ends inside an instruction")
        self.assertTrue(decoded)
        for got, want in decoded:
            self.assertEqual(got, want)


if __name__ == "__main__":
    unittest.main()
