"""Tests for game_files: the backup and restore of the registry key, the
files they leave alone, the settings.ini line a run sets, and song.ini
reads.

Every test runs on temp folders and a fake reg.exe; the real game files and
the real registry key are never read.
"""

from __future__ import annotations

import json
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
    "profiles.ini": b"[profile]\nplayer_name = Tester\n",
    "profiles.bin": b"\x06\x07\x08",
}
# The files the game saves on its own, by the folder it keeps them in. No
# backup or restore may touch them.
GAME_SAVES_IN_DOCUMENTS = ("scorestats.json", "profiles.ini", "profiles.bin")
GAME_SAVES_IN_LOCALLOW = ("scoredata.bin", "scoresext.bin", "user.bin")
GAME_SAVES = GAME_SAVES_IN_DOCUMENTS + GAME_SAVES_IN_LOCALLOW


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
        folder = self.low if name in GAME_SAVES_IN_LOCALLOW else self.docs
        return os.path.join(folder, name)

    def game_run(self) -> None:
        """What a run of the game does to its own saves: rewrite some, delete
        one and create one."""
        _write(self.path("scorestats.json"), b'{"score": 2}')
        _write(self.path("profiles.ini"), b"[profile]\nplayer_name = clone\n")
        _write(self.path("profiles.bin"), b"clone")
        os.remove(self.path("scoredata.bin"))
        _write(self.path("scoresext.bin"), b"new ext")
        _write(self.path("user.bin"), b"new user")

    def game_run_bytes(self) -> dict:
        return {name: (_read(self.path(name)) if os.path.exists(self.path(name)) else None)
                for name in GAME_SAVES}


def _old_backup(f: Folders) -> str:
    """A backup in the shape the tools once took: every game save copied and
    named in the manifest beside settings.ini."""
    folder = os.path.join(f.root, G.BACKUP_PREFIX + "old")
    os.makedirs(folder)
    entries = []
    for name in ("settings.ini",) + GAME_SAVES:
        source = os.path.abspath(f.path(name))
        existed = os.path.isfile(source)
        if existed:
            _write(os.path.join(folder, name), _read(source))
        entries.append({"name": name, "source": source, "existed": existed})
    with open(os.path.join(folder, G.BACKUP_MANIFEST), "w", encoding="utf-8") as fh:
        json.dump({"files": entries}, fh)
    return folder


class BackupRestoreTest(unittest.TestCase):
    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.tmp = self._tmp.name

    def tearDown(self):
        self._tmp.cleanup()

    def test_backup_copies_no_file(self):
        # The user's rulings of 2026-11-13: no file of the game's is backed up.
        f = Folders(self.tmp)
        backup = G.backup(f.root, stamp=lambda: "20261028-101500")
        self.assertEqual(backup, os.path.join(f.root, "backup-20261028-101500"))
        self.assertEqual(os.listdir(backup), [G.BACKUP_MANIFEST])
        with open(os.path.join(backup, G.BACKUP_MANIFEST), encoding="utf-8") as fh:
            self.assertEqual(json.load(fh)["files"], [])

    def test_restore_leaves_settings_ini_and_every_game_save_as_they_are(self):
        f = Folders(self.tmp, absent=("user.bin",))
        backup = G.backup(f.root, stamp=lambda: "s")
        _write(f.path("settings.ini"), b"changed")
        f.game_run()
        after_run = f.game_run_bytes()

        lines = G.restore_all(backup, run_reg=None, out=lambda _l: None)
        self.assertEqual(lines, [])
        self.assertEqual(_read(f.path("settings.ini")), b"changed")
        self.assertEqual(f.game_run_bytes(), after_run)
        self.assertEqual(os.listdir(backup), [G.BACKUP_MANIFEST])

    def test_an_older_backup_that_names_files_leaves_every_one_alone(self):
        f = Folders(self.tmp)
        backup = _old_backup(f)
        _write(f.path("settings.ini"), b"changed")
        f.game_run()
        after_run = f.game_run_bytes()
        in_backup = sorted(os.listdir(backup))

        seen = []
        lines = G.restore_all(backup, run_reg=None, out=seen.append)
        self.assertEqual(lines, [f"{name}: left as the game saved it"
                                 for name in ("settings.ini",) + GAME_SAVES])
        self.assertEqual(seen, lines)
        self.assertEqual(_read(f.path("settings.ini")), b"changed")
        self.assertEqual(f.game_run_bytes(), after_run)
        self.assertEqual(sorted(os.listdir(backup)), in_backup)

    def test_backup_refuses_an_existing_folder(self):
        f = Folders(self.tmp)
        G.backup(f.root, stamp=lambda: "same")
        with self.assertRaises(FileExistsError):
            G.backup(f.root, stamp=lambda: "same")


class FakeReg:
    """reg.exe on one fake key. export writes the key's text to the file,
    delete removes the key, import reads a file back into it. With
    `import_works` False an import leaves the key holding other text."""

    def __init__(self, text: bytes = b"Windows Registry Editor\r\nRewiredSaveData = 1\r\n"):
        self.text = text
        self.import_works = True
        self.calls = []

    def __call__(self, args):
        args = list(args)
        self.calls.append(args)
        verb = args[0]
        if args[1:2] and verb in ("export", "delete") and args[1] != G.REGISTRY_KEY:
            raise OSError(f"fake reg: unknown key {args[1]}")
        if verb == "export":
            if self.text is None:
                raise OSError("fake reg: the key does not exist")
            _write(args[2], self.text)
        elif verb == "delete":
            self.text = None
        elif verb == "import":
            self.text = _read(args[1]) if self.import_works else b"not what was imported"
        else:
            raise OSError(f"fake reg: unknown verb {verb}")

    def verbs(self):
        return [c[0] for c in self.calls]


class RegistryTest(unittest.TestCase):
    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.tmp = self._tmp.name
        self.f = Folders(self.tmp)
        self.reg = FakeReg()
        self.original = self.reg.text

    def tearDown(self):
        self._tmp.cleanup()

    def backup(self):
        return G.backup(self.f.root, stamp=lambda: "s", run_reg=self.reg)

    def test_the_key_is_rewireds_save_key(self):
        self.assertEqual(G.REGISTRY_KEY, r"HKCU\Software\srylain Inc.\Clone Hero")
        self.assertEqual(G.REGISTRY_BACKUP, "rewired.reg")

    def test_backup_exports_the_key_and_records_it(self):
        backup = self.backup()
        saved = os.path.join(backup, "rewired.reg")
        self.assertEqual(_read(saved), self.original)
        self.assertEqual(self.reg.calls, [["export", G.REGISTRY_KEY, saved, "/y"]])
        with open(os.path.join(backup, G.BACKUP_MANIFEST), encoding="utf-8") as f:
            entries = json.load(f)["files"]
        self.assertEqual(entries, [{"name": "rewired.reg", "key": G.REGISTRY_KEY}])

    def test_backup_with_no_runner_leaves_the_key_out(self):
        backup = G.backup(self.f.root, stamp=lambda: "s")
        self.assertFalse(os.path.exists(os.path.join(backup, "rewired.reg")))
        self.assertEqual(G.restore_registry(backup, run_reg=None, out=lambda _l: None), [])

    def test_an_unchanged_key_is_only_exported_and_compared(self):
        backup = self.backup()
        seen = []
        self.assertEqual(G.restore_registry(backup, run_reg=self.reg, out=seen.append),
                         ["rewired registry: unchanged"])
        self.assertEqual(seen, ["rewired registry: unchanged"])
        self.assertEqual(self.reg.verbs(), ["export", "export"])

    def test_a_changed_key_is_deleted_imported_and_checked(self):
        backup = self.backup()
        self.reg.text = b"the game changed this"
        lines = G.restore_registry(backup, run_reg=self.reg, out=lambda _l: None)
        self.assertEqual(lines, ["rewired registry: restored"])
        saved = os.path.join(backup, "rewired.reg")
        self.assertEqual(self.reg.calls[1:], [
            ["export", G.REGISTRY_KEY, saved + G.AFTER_RUN_SUFFIX, "/y"],
            ["delete", G.REGISTRY_KEY, "/f"],
            ["import", saved],
            ["export", G.REGISTRY_KEY, saved + G.REGISTRY_CHECK_SUFFIX, "/y"],
        ])
        self.assertEqual(self.reg.text, self.original)
        # The game's version stays in the backup folder.
        self.assertEqual(_read(saved + G.AFTER_RUN_SUFFIX), b"the game changed this")

    def test_a_key_that_still_differs_after_the_import_raises(self):
        backup = self.backup()
        self.reg.text = b"the game changed this"
        self.reg.import_works = False
        with self.assertRaises(G.RestoreError) as cm:
            G.restore_registry(backup, run_reg=self.reg, out=lambda _l: None)
        self.assertEqual(cm.exception.name, "rewired.reg")
        self.assertIn("still differs", str(cm.exception))

    def test_a_failing_reg_exe_raises_with_the_name(self):
        backup = self.backup()
        self.reg.text = None  # the key is gone, so the export fails
        with self.assertRaises(G.RestoreError) as cm:
            G.restore_registry(backup, run_reg=self.reg, out=lambda _l: None)
        self.assertEqual(cm.exception.name, "rewired.reg")

    def test_a_backup_with_the_key_and_no_runner_refuses(self):
        backup = self.backup()
        with self.assertRaises(G.RestoreError) as cm:
            G.restore_registry(backup, run_reg=None, out=lambda _l: None)
        self.assertEqual(cm.exception.name, "rewired.reg")

    def test_the_left_as_saved_lines_skip_the_registry_entry(self):
        backup = self.backup()
        self.assertEqual(G.left_as_saved(backup, out=lambda _l: None), [])
        self.assertEqual(self.reg.verbs(), ["export"])

    def test_restore_all_puts_the_key_back(self):
        backup = self.backup()
        lines = G.restore_all(backup, run_reg=self.reg, out=lambda _l: None)
        self.assertEqual(lines, ["rewired registry: unchanged"])
        self.assertEqual(self.reg.verbs(), ["export", "export"])

    def test_restore_all_refuses_a_backup_with_the_key_when_given_no_runner(self):
        backup = self.backup()
        with self.assertRaises(G.RestoreError) as cm:
            G.restore_all(backup, run_reg=None, out=lambda _l: None)
        self.assertEqual(cm.exception.name, "rewired.reg")
        self.assertEqual(self.reg.verbs(), ["export"])

    def test_reg_exe_gets_the_users_30_second_cap(self):
        self.assertEqual(G.REG_CAP_S, 30.0)
        seen = {}

        def fake_run(cmd, **kw):
            seen.update(kw)
            raise G.subprocess.TimeoutExpired(cmd, kw["timeout"])

        with mock.patch.object(G.subprocess, "run", fake_run):
            with self.assertRaises(OSError) as cm:
                G.run_reg(["export", G.REGISTRY_KEY, "x.reg", "/y"])
        self.assertEqual(seen["timeout"], 30.0)
        self.assertIn("did not finish in 30 s", str(cm.exception))

    def test_a_reg_exe_timeout_fails_the_backup_and_the_restore_by_name(self):
        def timing_out(_args):
            raise OSError("reg.exe did not finish in 30 s")

        with self.assertRaises(OSError):
            G.backup(self.f.root, stamp=lambda: "t", run_reg=timing_out)
        backup = self.backup()
        with self.assertRaises(G.RestoreError) as cm:
            G.restore_all(backup, run_reg=timing_out, out=lambda _l: None)
        self.assertEqual(cm.exception.name, "rewired.reg")


# Shaped like the user's settings.ini: a [game] section holding the line,
# with sections before and after it and Windows line endings.
SETTINGS_FIXTURE = (b"[audio]\r\nmaster_volume = 0.8\r\npause_on_focus_lost = 1\r\n\r\n"
                    b"[game]\r\nnote_speed = 7\r\npause_on_focus_lost = 1\r\n"
                    b"highway_length = 1\r\n\r\n[video]\r\nfps = 0\r\n")


class SetGameSettingTest(unittest.TestCase):
    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.path = os.path.join(self._tmp.name, "settings.ini")

    def tearDown(self):
        self._tmp.cleanup()

    def test_the_run_line_is_pause_on_focus_lost_0_in_game(self):
        self.assertEqual((G.GAME_SECTION, G.PAUSE_ON_FOCUS_LOST, G.PAUSE_ON_FOCUS_LOST_RUN_VALUE),
                         ("game", "pause_on_focus_lost", "0"))

    def test_changes_exactly_the_line_in_the_game_section(self):
        _write(self.path, SETTINGS_FIXTURE)
        G.set_game_setting(self.path, "pause_on_focus_lost", "0")
        self.assertEqual(_read(self.path), SETTINGS_FIXTURE.replace(
            b"note_speed = 7\r\npause_on_focus_lost = 1\r\n",
            b"note_speed = 7\r\npause_on_focus_lost = 0\r\n"))

    def test_keeps_the_lines_own_spacing_and_a_missing_last_newline(self):
        _write(self.path, b"\xef\xbb\xbf[Game]\nPause_On_Focus_Lost=1")
        G.set_game_setting(self.path, "pause_on_focus_lost", "0")
        self.assertEqual(_read(self.path), b"\xef\xbb\xbf[Game]\nPause_On_Focus_Lost=0")

    def test_refuses_when_the_key_is_absent(self):
        before = SETTINGS_FIXTURE.replace(b"note_speed = 7\r\npause_on_focus_lost = 1\r\n",
                                          b"note_speed = 7\r\n")
        _write(self.path, before)
        with self.assertRaises(G.SettingNotFound):
            G.set_game_setting(self.path, "pause_on_focus_lost", "0")
        self.assertEqual(_read(self.path), before)

    def test_refuses_when_the_key_sits_only_in_another_section(self):
        before = b"[audio]\r\npause_on_focus_lost = 1\r\n[game]\r\nnote_speed = 7\r\n"
        _write(self.path, before)
        with self.assertRaises(G.SettingNotFound):
            G.set_game_setting(self.path, "pause_on_focus_lost", "0")
        self.assertEqual(_read(self.path), before)

    def test_refuses_two_lines_for_the_key(self):
        before = b"[game]\npause_on_focus_lost = 1\npause_on_focus_lost = 1\n"
        _write(self.path, before)
        with self.assertRaises(G.SettingNotFound):
            G.set_game_setting(self.path, "pause_on_focus_lost", "0")
        self.assertEqual(_read(self.path), before)


class GetGameSettingTest(unittest.TestCase):
    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.path = os.path.join(self._tmp.name, "settings.ini")

    def tearDown(self):
        self._tmp.cleanup()

    def test_reads_the_value_of_the_line_in_the_game_section(self):
        _write(self.path, SETTINGS_FIXTURE.replace(b"note_speed = 7\r\npause_on_focus_lost = 1\r\n",
                                                   b"note_speed = 7\r\npause_on_focus_lost = 0\r\n"))
        self.assertEqual(G.get_game_setting(self.path, "pause_on_focus_lost"), "0")

    def test_drops_the_spacing_and_the_line_ending(self):
        _write(self.path, b"\xef\xbb\xbf[Game]\nPause_On_Focus_Lost =  1  \r\n")
        self.assertEqual(G.get_game_setting(self.path, "pause_on_focus_lost"), "1")

    def test_refuses_when_the_key_sits_only_in_another_section(self):
        _write(self.path, b"[audio]\r\npause_on_focus_lost = 1\r\n[game]\r\nnote_speed = 7\r\n")
        with self.assertRaises(G.SettingNotFound):
            G.get_game_setting(self.path, "pause_on_focus_lost")


class SongIniTest(unittest.TestCase):
    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.tmp = self._tmp.name

    def tearDown(self):
        self._tmp.cleanup()

    def _ini(self, text: str) -> str:
        _write(os.path.join(self.tmp, G.SONG_INI), text.encode("utf-8"))
        return self.tmp

    def test_reads_pro_drums_from_the_song_section(self):
        d = self._ini("﻿[Song]\nname = X\npro_drums = True\n")
        self.assertTrue(G.song_ini_pro_drums(d))

    def test_other_sections_do_not_count(self):
        d = self._ini("[other]\npro_drums = True\n[song]\npro_drums = False\n")
        self.assertFalse(G.song_ini_pro_drums(d))

    def test_the_song_ini_length_reader_is_gone(self):
        # The song's length is Hydra's end, read through hydra_end (D321 item 2).
        self.assertFalse(hasattr(G, "song_ini_length_ms"))


if __name__ == "__main__":
    unittest.main()
