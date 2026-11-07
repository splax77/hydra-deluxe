"""Tests for tools/ch_mcp/launch_patch.json, the one-byte patch the launch
agent makes in memory, checked against the rest of the repo: the facts table
and the loader's listing that task D1 of the direct-start plan wrote
(docs/superpowers/plans/2026-10-31-ch-mcp-direct-start.md, task D5).

Every expected value below is a literal read once by hand from the listing or
the DLL; none is worked out again from the file under test. The game is not in
the repo, so the bytes the agent compares against the game's memory are pinned
here, and the facts table's header md5 is pinned beside them: a new
GameAssembly.dll changes that md5, and this test then fails until someone reads
the bytes again.
"""

from __future__ import annotations

import csv
import json
import os
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

# The md5 of the GameAssembly.dll the bytes below were read from, as the facts
# table's header names it.
DLL_MD5_LINE = "# GameAssembly.dll md5: 106e15e5bf799ea62f916e8ceaf38bc6"
# The command-line player loader and the instruction the patch changes.
LOADER_RVA = "0x3968D0"
PATCH_RVA = "0x396EC5"
# The loader's first twelve bytes, read from that DLL's file at the loader's
# offset by the direct-start plan's reviewer: push rsi (40 56), push rdi (57),
# push r13 (41 55), sub rsp, 0xc0 (48 81 EC C0 00 00 00).
ENTRY_EXPECT = "40 56 57 41 55 48 81 EC C0 00 00 00"
# mov edx, 1 and mov edx, 0.
EXPECT = "BA 01 00 00 00"
WRITE = "BA 00 00 00 00"
# The listing's first four lines, which the twelve bytes above encode.
LISTING_OPENING = [
    "003968d0: push   rsi",
    "003968d2: push   rdi",
    "003968d3: push   r13",
    "003968d5: sub    rsp, 0xc0",
]
# The listing's line at the patch RVA.
LISTING_PATCH_LINE = "00396ec5: mov    edx, 1"


def _load_patch() -> dict:
    with open(PATCH_FILE, encoding="utf-8") as f:
        return json.load(f)


def _listing_lines() -> list[str]:
    with open(LISTING_FILE, encoding="utf-8") as f:
        return [line.rstrip("\n") for line in f]


def _facts_rows() -> tuple[list[str], list[dict]]:
    """The facts table's comment lines and its rows."""
    with open(FACTS_FILE, encoding="utf-8", newline="") as f:
        lines = f.read().splitlines()
    comments = [line for line in lines if line.startswith("#")]
    body = [line for line in lines if not line.startswith("#")]
    return comments, list(csv.DictReader(body, delimiter="\t"))


class LaunchPatchFileTest(unittest.TestCase):
    def setUp(self):
        self.patch = _load_patch()

    def test_the_file_parses_as_one_object(self):
        self.assertIsInstance(self.patch, dict)

    def test_it_patches_the_game_assembly_module(self):
        self.assertEqual(self.patch["module"], "GameAssembly.dll")

    def test_launch_and_defer_hold_values_the_launch_knows(self):
        # The live steps change these two between runs, so only their kind is
        # pinned.
        self.assertIn(self.patch["launch"], bot_run.LAUNCH_FORMS)
        self.assertIsInstance(self.patch["defer"], bool)

    def test_the_addresses_are_the_loader_and_its_bot_write(self):
        self.assertEqual(self.patch["loader_rva"], LOADER_RVA)
        self.assertEqual(self.patch["patch_rva"], PATCH_RVA)

    def test_expect_is_mov_edx_1(self):
        self.assertEqual(self.patch["expect"], EXPECT)

    def test_write_is_mov_edx_0(self):
        self.assertEqual(self.patch["write"], WRITE)

    def test_write_differs_from_expect_in_the_one_byte_at_plus_one(self):
        expect = bytes.fromhex(self.patch["expect"])
        write = bytes.fromhex(self.patch["write"])
        self.assertEqual(len(write), len(expect))
        differ = [i for i in range(len(expect)) if expect[i] != write[i]]
        self.assertEqual(differ, [1])

    def test_entry_expect_is_the_loaders_first_twelve_bytes(self):
        self.assertEqual(self.patch["entry_expect"], ENTRY_EXPECT)


class AgainstTheRepoTest(unittest.TestCase):
    def setUp(self):
        self.patch = _load_patch()

    def test_the_bytes_were_read_from_the_dll_the_facts_table_names(self):
        comments, _ = _facts_rows()
        self.assertIn(DLL_MD5_LINE, comments)

    def test_the_loader_rva_has_a_method_row_in_the_facts_table(self):
        _, rows = _facts_rows()
        methods = [r for r in rows if r["kind"] == "method" and r["rva"] == LOADER_RVA]
        self.assertEqual(len(methods), 1, methods)
        self.assertEqual(self.patch["loader_rva"], LOADER_RVA)

    def test_the_listing_reads_mov_edx_1_at_the_patch_rva(self):
        self.assertIn(LISTING_PATCH_LINE, _listing_lines())
        self.assertEqual(self.patch["patch_rva"], PATCH_RVA)

    def test_the_listing_opens_with_the_instructions_entry_expect_encodes(self):
        self.assertEqual(_listing_lines()[:4], LISTING_OPENING)
        self.assertEqual(self.patch["entry_expect"], ENTRY_EXPECT)


if __name__ == "__main__":
    unittest.main()
