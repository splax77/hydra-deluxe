"""Tests for game_files: the user-file backup and restore, and song.ini reads.

Every test runs on temp folders; the real game files are never read.
"""

from __future__ import annotations

import os
import sys
import tempfile
import unittest
from unittest import mock

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_mcp import game_files as G  # noqa: E402

ORIGINAL = {
    "settings.ini": b"[game]\nvolume = 1\n",
    "scorestats.json": b'{"score": 1}',
    "scoredata.bin": b"\x00\x01\x02",
    "scoresext.bin": b"\x03\x04",
    "user.bin": b"\x05" * 64,
}


def _write(path: str, data: bytes) -> None:
    with open(path, "wb") as f:
        f.write(data)


def _read(path: str) -> bytes:
    with open(path, "rb") as f:
        return f.read()


class Folders:
    """A temp Documents folder, LocalLow folder and backup root."""

    def __init__(self, tmp: str, absent=()):
        self.docs = os.path.join(tmp, "Documents", "Clone Hero")
        self.low = os.path.join(tmp, "LocalLow", "Clone Hero")
        self.root = os.path.join(tmp, "backups")
        os.makedirs(self.docs)
        os.makedirs(self.low)
        for name, data in ORIGINAL.items():
            if name not in absent:
                _write(self.path(name), data)

    def path(self, name: str) -> str:
        folder = self.docs if name in G.DOCUMENTS_FILES else self.low
        return os.path.join(folder, name)


class BackupRestoreTest(unittest.TestCase):
    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.tmp = self._tmp.name

    def tearDown(self):
        self._tmp.cleanup()

    def test_the_five_files_are_the_safety_rule_list(self):
        self.assertEqual(sorted(G.DOCUMENTS_FILES + G.LOCALLOW_FILES), sorted(ORIGINAL))

    def test_round_trip_restores_all_five_byte_for_byte(self):
        f = Folders(self.tmp, absent=("user.bin",))
        backup = G.backup(f.docs, f.low, f.root, stamp=lambda: "20261028-101500")
        self.assertEqual(backup, os.path.join(f.root, "backup-20261028-101500"))

        # The run: the game rewrites three files, leaves one, and creates the
        # one that was absent.
        _write(f.path("settings.ini"), b"changed")
        _write(f.path("scorestats.json"), b'{"score": 2}')
        os.remove(f.path("scoredata.bin"))
        _write(f.path("user.bin"), b"new")

        lines = G.restore(backup, out=lambda _l: None)
        for name in ("settings.ini", "scorestats.json", "scoredata.bin", "scoresext.bin"):
            self.assertEqual(_read(f.path(name)), ORIGINAL[name], name)
        self.assertFalse(os.path.exists(f.path("user.bin")))
        aside = os.path.join(backup, "user.bin" + G.AFTER_RUN_SUFFIX)
        self.assertEqual(_read(aside), b"new")
        self.assertEqual(lines, [
            "settings.ini: restored",
            "scorestats.json: restored",
            "scoredata.bin: restored",
            "scoresext.bin: unchanged",
            f"user.bin: absent before the run; the game's new copy moved to {aside}",
        ])

    def test_a_file_absent_before_and_after_is_reported_unchanged(self):
        f = Folders(self.tmp, absent=("scoresext.bin",))
        backup = G.backup(f.docs, f.low, f.root, stamp=lambda: "s")
        lines = G.restore(backup, out=lambda _l: None)
        self.assertIn("scoresext.bin: unchanged (absent before and after the run)", lines)
        self.assertFalse(os.path.exists(f.path("scoresext.bin")))

    def test_a_failed_restore_stops_with_the_file_name(self):
        f = Folders(self.tmp)
        backup = G.backup(f.docs, f.low, f.root, stamp=lambda: "s")
        _write(f.path("settings.ini"), b"changed")
        _write(f.path("user.bin"), b"changed")
        real_copy = G.shutil.copy2

        def copy2(src, dst, *a, **k):
            if os.path.basename(dst) == "settings.ini":
                raise PermissionError("locked")
            return real_copy(src, dst, *a, **k)

        seen = []
        with mock.patch.object(G.shutil, "copy2", copy2):
            with self.assertRaises(G.RestoreError) as cm:
                G.restore(backup, out=seen.append)
        self.assertEqual(cm.exception.name, "settings.ini")
        self.assertIn("settings.ini", str(cm.exception))
        self.assertIn("locked", str(cm.exception))
        # It stopped there: nothing was printed and the later file is untouched.
        self.assertEqual(seen, [])
        self.assertEqual(_read(f.path("user.bin")), b"changed")

    def test_backup_refuses_an_existing_folder(self):
        f = Folders(self.tmp)
        G.backup(f.docs, f.low, f.root, stamp=lambda: "same")
        with self.assertRaises(FileExistsError):
            G.backup(f.docs, f.low, f.root, stamp=lambda: "same")


class SongIniTest(unittest.TestCase):
    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.tmp = self._tmp.name

    def tearDown(self):
        self._tmp.cleanup()

    def _ini(self, text: str) -> str:
        _write(os.path.join(self.tmp, G.SONG_INI), text.encode("utf-8"))
        return self.tmp

    def test_reads_length_and_pro_drums_from_the_song_section(self):
        d = self._ini("﻿[Song]\nname = X\nSong_Length = 62080\npro_drums = True\n")
        self.assertEqual(G.song_ini_length_ms(d), 62080)
        self.assertTrue(G.song_ini_pro_drums(d))

    def test_other_sections_do_not_count(self):
        d = self._ini("[other]\nsong_length = 5\n[song]\npro_drums = False\n")
        self.assertIsNone(G.song_ini_length_ms(d))
        self.assertFalse(G.song_ini_pro_drums(d))

    def test_bad_or_missing_length_is_none(self):
        self.assertIsNone(G.song_ini_length_ms(self.tmp))
        self.assertIsNone(G.song_ini_length_ms(self._ini("[song]\nsong_length = abc\n")))
        self.assertIsNone(G.song_ini_length_ms(self._ini("[song]\nsong_length = 0\n")))


if __name__ == "__main__":
    unittest.main()
