"""Tests for game_files: the backup and restore of the registry key, the
files they leave alone, the settings.ini line a run sets, and song.ini
reads.

The backup and restore tests run on temp folders, real backups and key
exports from earlier runs, and the real reg.exe. The game's own key is only
ever exported (read). Every key a test changes is a scratch key under
HKCU\\Software that the test makes and deletes. FakeReg stays for the
test_game_tools tests that still drive restore_registry with it.
"""

from __future__ import annotations

import json
import os
import shutil
import subprocess
import sys
import tempfile
import unittest
import uuid
from unittest import mock

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_mcp import game_files as G  # noqa: E402
from tools.ch_mcp.tests.test_bot_run import (  # noqa: E402
    REAL_BACKUP, STATS_AFTER_A_RUN, _copy_real_backup, _folder_bytes)

# Trimmed from the game key's export before and after a real run (copied from
# hydra-data\ch-mcp-runs\2026-11-12\ls-g1-run1\backup-20261112-182956,
# rewired.reg and rewired.reg.after-run): the header, the key's line and the
# two values that run changed (lines 1-3 and 20-22), in reg.exe's own UTF-16.
REAL_KEY_EXPORTS = os.path.join(_REPO_ROOT, "testdata", "ch_runs", "ls-g1-run1", "rewired-key")
KEY_BEFORE_A_RUN = os.path.join(REAL_KEY_EXPORTS, G.REGISTRY_BACKUP)
KEY_AFTER_A_RUN = os.path.join(REAL_KEY_EXPORTS, G.REGISTRY_BACKUP + G.AFTER_RUN_SUFFIX)


def _write(path: str, data: bytes) -> None:
    with open(path, "wb") as f:
        f.write(data)


def _read(path: str) -> bytes:
    with open(path, "rb") as f:
        return f.read()


def _reg_text(path: str) -> str:
    """A .reg file reg.exe wrote, as text."""
    return _read(path).decode("utf-16")


def _key_line(reg_text: str) -> str:
    """The one bracketed line of a .reg text that holds a single key: the
    key's full name as reg.exe writes it."""
    lines = [line for line in reg_text.splitlines() if line.startswith("[")]
    if len(lines) != 1:
        raise AssertionError(f"expected one key line, got {lines}")
    return lines[0]


def _scratch_key(test: unittest.TestCase, tmp: str) -> tuple[str, str]:
    """A new empty key under HKCU\\Software, made with the real reg.exe and
    deleted when the test ends. Returns its name for reg.exe and its key
    line as reg.exe exports it."""
    key = r"HKCU\Software\HydraTests-game_files-" + uuid.uuid4().hex
    G.run_reg(["add", key, "/f"])
    test.addCleanup(G.run_reg, ["delete", key, "/f"])
    probe = os.path.join(tmp, "scratch-key-line.reg")
    G.run_reg(["export", key, probe, "/y"])
    line = _key_line(_reg_text(probe))
    os.remove(probe)
    return key, line


def _real_values_for(key_line: str, recorded: str) -> str:
    """The recorded export's text with its key line swapped for `key_line`,
    so an import of it writes the run's real values into that key and never
    into the game's."""
    text = _reg_text(recorded)
    game_line = _key_line(text)
    if game_line == key_line:
        raise AssertionError("the scratch key is the game's key")
    return text.replace(game_line, key_line)


def _import_values(tmp: str, key_line: str, recorded: str) -> None:
    path = os.path.join(tmp, "import-" + uuid.uuid4().hex + ".reg")
    _write(path, _real_values_for(key_line, recorded).encode("utf-16"))
    G.run_reg(["import", path])


def _scratch_backup(test: unittest.TestCase, tmp: str, recorded: str = KEY_BEFORE_A_RUN):
    """A backup folder in the shape backup() writes when it holds the key,
    for a scratch key that holds `recorded`'s real values: its manifest names
    the scratch key, and REGISTRY_BACKUP is the real reg.exe's export of it.
    Returns the folder, the key and its key line."""
    key, line = _scratch_key(test, tmp)
    _import_values(tmp, line, recorded)
    folder = os.path.join(tmp, G.BACKUP_PREFIX + uuid.uuid4().hex)
    os.makedirs(folder)
    G.run_reg(["export", key, os.path.join(folder, G.REGISTRY_BACKUP), "/y"])
    with open(os.path.join(folder, G.BACKUP_MANIFEST), "w", encoding="utf-8") as f:
        json.dump({"files": [{"name": G.REGISTRY_BACKUP, "key": key}]}, f)
    return folder, key, line


def _key_entries(backup: str) -> list:
    with open(os.path.join(backup, G.BACKUP_MANIFEST), encoding="utf-8") as f:
        return [e for e in json.load(f)["files"] if "key" in e]


def _file_entries(backup: str) -> list:
    with open(os.path.join(backup, G.BACKUP_MANIFEST), encoding="utf-8") as f:
        return [e for e in json.load(f)["files"] if "key" not in e]


class BackupRestoreTest(unittest.TestCase):
    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.tmp = self._tmp.name
        self.root = os.path.join(self.tmp, "backups")

    def tearDown(self):
        self._tmp.cleanup()

    def test_backup_copies_no_file(self):
        # The user's rulings of 2026-11-13: no file of the game's is backed up.
        backup = G.backup(self.root)
        self.assertEqual(os.listdir(self.root), [os.path.basename(backup)])
        self.assertTrue(os.path.basename(backup).startswith(G.BACKUP_PREFIX), backup)
        self.assertEqual(os.listdir(backup), [G.BACKUP_MANIFEST])
        with open(os.path.join(backup, G.BACKUP_MANIFEST), encoding="utf-8") as fh:
            self.assertEqual(json.load(fh)["files"], [])

    def test_restore_of_a_new_backup_reaches_no_file(self):
        # A backup names in its manifest every file a restore may reach, and
        # a new one names none, so the restore prints nothing and changes
        # nothing, its own folder included.
        backup = G.backup(self.root)
        before = _folder_bytes(backup)
        seen = []
        self.assertEqual(G.restore_all(backup, run_reg=None, out=seen.append), [])
        self.assertEqual(seen, [])
        self.assertEqual(_folder_bytes(backup), before)

    def test_an_older_backup_that_names_files_leaves_every_one_alone(self):
        # A real backup from before the rulings of 2026-11-13 (REAL_BACKUP),
        # its file sources moved under the temp folder and its key entry
        # pointed at a scratch key, so the restore runs whole with the real
        # reg.exe and never reaches the user's files or the game's key. The
        # files at the sources are what a run left: settings.ini with the
        # run's line set, and scorestats.json as the game wrote it after a run.
        backup, sources = _copy_real_backup(self.tmp)
        scratch, key, _line = _scratch_backup(self, self.tmp)
        manifest = os.path.join(backup, G.BACKUP_MANIFEST)
        with open(manifest, encoding="utf-8") as f:
            doc = json.load(f)
        for e in doc["files"]:
            if "key" in e:
                e["key"] = key
        with open(manifest, "w", encoding="utf-8") as f:
            json.dump(doc, f)
        shutil.copyfile(os.path.join(scratch, G.REGISTRY_BACKUP),
                        os.path.join(backup, G.REGISTRY_BACKUP))
        shutil.copyfile(os.path.join(backup, G.SETTINGS), sources[G.SETTINGS])
        G.set_game_setting(sources[G.SETTINGS], G.PAUSE_ON_FOCUS_LOST,
                           G.PAUSE_ON_FOCUS_LOST_RUN_VALUE)
        shutil.copyfile(STATS_AFTER_A_RUN, sources[G.SCORESTATS])
        run_left = _folder_bytes(os.path.dirname(sources[G.SETTINGS]))
        backup_before = _folder_bytes(backup)

        seen = []
        lines = G.restore_all(backup, run_reg=G.run_reg, out=seen.append)
        self.assertEqual(seen, lines)
        # One line for each file the manifest names, saying it was left alone,
        # then the key's line.
        names = [e["name"] for e in _file_entries(backup)]
        self.assertEqual(len(lines), len(names) + 1, lines)
        for name, line in zip(names, lines):
            self.assertTrue(line.startswith(name), line)
            self.assertIn(G.LEFT_AS_SAVED, line)
        self.assertTrue(lines[-1].startswith(G.REGISTRY_LINE_NAME), lines)
        self.assertEqual(_folder_bytes(os.path.dirname(sources[G.SETTINGS])), run_left)
        # The backup keeps every file it held; the restore adds only the
        # key's export after the run.
        after = _folder_bytes(backup)
        self.assertEqual(sorted(set(after) - set(backup_before)),
                         [G.REGISTRY_BACKUP + G.AFTER_RUN_SUFFIX])
        self.assertEqual({n: after[n] for n in backup_before}, backup_before)

    def test_backup_refuses_an_existing_folder(self):
        G.backup(self.root, stamp=lambda: "same")
        with self.assertRaises(FileExistsError):
            G.backup(self.root, stamp=lambda: "same")


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


class RealRegistryTest(unittest.TestCase):
    """backup() and restore_registry() with the real reg.exe. The game's key
    is only exported; restores run on scratch keys."""

    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.tmp = self._tmp.name
        self.root = os.path.join(self.tmp, "backups")

    def tearDown(self):
        self._tmp.cleanup()

    def test_the_key_backed_up_is_the_one_the_game_saved_under(self):
        # The key's export from a real run's backup names the key the game
        # saved Rewired's bindings under; a backup now exports that same key.
        backup = G.backup(self.root, run_reg=G.run_reg)
        self.assertEqual(_key_line(_reg_text(os.path.join(backup, G.REGISTRY_BACKUP))),
                         _key_line(_reg_text(KEY_BEFORE_A_RUN)))

    def test_backup_exports_the_key_and_records_it(self):
        backup = G.backup(self.root, run_reg=G.run_reg)
        # The manifest names the export and the key it came from, and the
        # export is the only other file in the folder.
        self.assertEqual(_key_entries(backup), [{"name": G.REGISTRY_BACKUP, "key": G.REGISTRY_KEY}])
        self.assertEqual(_file_entries(backup), [])
        self.assertEqual(sorted(os.listdir(backup)),
                         sorted([G.BACKUP_MANIFEST, G.REGISTRY_BACKUP]))

    def test_backup_with_no_runner_leaves_the_key_out(self):
        backup = G.backup(self.root)
        self.assertFalse(os.path.exists(os.path.join(backup, G.REGISTRY_BACKUP)))
        self.assertEqual(_key_entries(backup), [])
        self.assertEqual(G.restore_registry(backup, run_reg=None, out=lambda _l: None), [])

    def test_an_unchanged_key_is_only_exported_and_compared(self):
        backup, key, _line = _scratch_backup(self, self.tmp)
        saved = os.path.join(backup, G.REGISTRY_BACKUP)
        seen = []
        lines = G.restore_registry(backup, run_reg=G.run_reg, out=seen.append)
        self.assertEqual(seen, lines)
        self.assertEqual(len(lines), 1, lines)
        self.assertTrue(lines[0].startswith(G.REGISTRY_LINE_NAME), lines)
        # One export, equal to the saved one; no import, so no check export.
        self.assertEqual(_read(saved + G.AFTER_RUN_SUFFIX), _read(saved))
        self.assertFalse(os.path.exists(saved + G.REGISTRY_CHECK_SUFFIX))
        # A key the game changed gets a different line.
        changed, _key, line = _scratch_backup(self, self.tmp)
        _import_values(self.tmp, line, KEY_AFTER_A_RUN)
        other = G.restore_registry(changed, run_reg=G.run_reg, out=lambda _l: None)
        self.assertEqual(len(other), 1, other)
        self.assertNotEqual(other, lines)

    def _changed_scratch_backup(self):
        """A scratch backup whose key then takes the values a real run left,
        as the game would change it. Returns the folder, the key, the saved
        export and the key's export as the run left it."""
        backup, key, line = _scratch_backup(self, self.tmp)
        saved = os.path.join(backup, G.REGISTRY_BACKUP)
        _import_values(self.tmp, line, KEY_AFTER_A_RUN)
        games = os.path.join(self.tmp, "games-version-" + uuid.uuid4().hex + ".reg")
        G.run_reg(["export", key, games, "/y"])
        self.assertNotEqual(_read(games), _read(saved), "the run's values changed nothing")
        return backup, key, saved, games

    def _assert_key_is(self, key: str, path: str):
        now = os.path.join(self.tmp, "key-now-" + uuid.uuid4().hex + ".reg")
        G.run_reg(["export", key, now, "/y"])
        self.assertEqual(_read(now), _read(path))

    def test_a_changed_key_is_put_back_and_the_games_version_kept(self):
        backup, key, saved, games = self._changed_scratch_backup()
        lines = G.restore_registry(backup, run_reg=G.run_reg, out=lambda _l: None)
        self.assertEqual(len(lines), 1, lines)
        self.assertTrue(lines[0].startswith(G.REGISTRY_LINE_NAME), lines)
        # The key holds the saved values again, and the backup folder keeps
        # the key as the game left it, so it was exported before any delete.
        self._assert_key_is(key, saved)
        self.assertEqual(_read(saved + G.AFTER_RUN_SUFFIX), _read(games))

    def test_restore_all_puts_a_changed_key_back(self):
        backup, key, saved, games = self._changed_scratch_backup()
        lines = G.restore_all(backup, run_reg=G.run_reg, out=lambda _l: None)
        self.assertEqual(len(lines), 1, lines)
        self.assertTrue(lines[0].startswith(G.REGISTRY_LINE_NAME), lines)
        self._assert_key_is(key, saved)
        self.assertEqual(_read(saved + G.AFTER_RUN_SUFFIX), _read(games))

    def test_a_key_that_still_differs_after_the_import_raises(self):
        # The scratch key holds the values a real run left; the saved export
        # holds the values from before it, written as REGEDIT4 text, which
        # reg.exe imports but exports again as its own UTF-16. So the key's
        # export after the import still differs from the saved bytes.
        backup, key, line = _scratch_backup(self, self.tmp, KEY_AFTER_A_RUN)
        saved = os.path.join(backup, G.REGISTRY_BACKUP)
        text = _real_values_for(line, KEY_BEFORE_A_RUN)
        header = text.splitlines()[0]
        _write(saved, text.replace(header, "REGEDIT4").encode("ascii"))
        with self.assertRaises(G.RestoreError) as cm:
            G.restore_registry(backup, run_reg=G.run_reg, out=lambda _l: None)
        self.assertEqual(cm.exception.name, _key_entries(backup)[0]["name"])
        # The error names the key, the saved export and where the game's
        # version of the key was kept, and that export is there.
        message = str(cm.exception)
        after_run = saved + G.AFTER_RUN_SUFFIX
        for part in (key, saved, after_run):
            self.assertIn(part, message)
        self.assertTrue(os.path.isfile(after_run))
        self.assertNotEqual(_read(saved + G.REGISTRY_CHECK_SUFFIX), _read(saved))

    def test_a_failing_reg_exe_raises_with_the_name(self):
        # The manifest names a key nobody made, so reg.exe's export of it
        # exits non-zero.
        missing = r"HKCU\Software\HydraTests-game_files-" + uuid.uuid4().hex
        backup = os.path.join(self.tmp, G.BACKUP_PREFIX + "missing-key")
        os.makedirs(backup)
        shutil.copyfile(KEY_BEFORE_A_RUN, os.path.join(backup, G.REGISTRY_BACKUP))
        with open(os.path.join(backup, G.BACKUP_MANIFEST), "w", encoding="utf-8") as f:
            json.dump({"files": [{"name": G.REGISTRY_BACKUP, "key": missing}]}, f)
        with self.assertRaises(G.RestoreError) as cm:
            G.restore_registry(backup, run_reg=G.run_reg, out=lambda _l: None)
        self.assertEqual(cm.exception.name, G.REGISTRY_BACKUP)
        self.assertIsInstance(cm.exception.error, OSError)
        self.assertIn(missing, str(cm.exception.error))
        self.assertFalse(os.path.exists(
            os.path.join(backup, G.REGISTRY_BACKUP + G.AFTER_RUN_SUFFIX)))

    def test_a_backup_with_the_key_and_no_runner_refuses(self):
        backup, _sources = _copy_real_backup(self.tmp)
        names = [e["name"] for e in _key_entries(backup)]
        self.assertEqual(len(names), 1, names)
        with self.assertRaises(G.RestoreError) as cm:
            G.restore_registry(backup, run_reg=None, out=lambda _l: None)
        self.assertEqual(cm.exception.name, names[0])

    def test_the_left_as_saved_lines_skip_the_registry_entry(self):
        # REAL_BACKUP's manifest names the game's files and the key.
        backup, sources = _copy_real_backup(self.tmp)
        before = _folder_bytes(backup)
        files = [e["name"] for e in _file_entries(backup)]
        key_names = [e["name"] for e in _key_entries(backup)]
        self.assertTrue(files and key_names, (files, key_names))
        lines = G.left_as_saved(backup, out=lambda _l: None)
        self.assertEqual(len(lines), len(files), lines)
        for name, line in zip(files, lines):
            self.assertTrue(line.startswith(name), line)
            self.assertIn(G.LEFT_AS_SAVED, line)
        for name in key_names:
            self.assertFalse(any(name in line for line in lines), lines)
        # Nothing was put back at any source, and the backup is as it was.
        self.assertFalse(any(os.path.exists(p) for p in sources.values()), sources)
        self.assertEqual(_folder_bytes(backup), before)

    def test_restore_all_refuses_a_backup_with_the_key_when_given_no_runner(self):
        # REAL_BACKUP holds the key. restore_all says what it left alone,
        # then refuses the key by name rather than leave it out without a word.
        backup, _sources = _copy_real_backup(self.tmp)
        before = _folder_bytes(backup)
        seen = []
        with self.assertRaises(G.RestoreError) as cm:
            G.restore_all(backup, run_reg=None, out=seen.append)
        self.assertEqual(cm.exception.name, _key_entries(backup)[0]["name"])
        self.assertEqual(seen, G.left_as_saved(backup, out=lambda _l: None))
        self.assertEqual(_folder_bytes(backup), before)

    def test_a_reg_exe_call_past_the_cap_fails_naming_the_command(self):
        # The real reg.exe, with REG_CAP_S cut below the time any reg.exe
        # call takes, so the cap is what stops it.
        path = os.path.join(self.tmp, "capped.reg")
        with mock.patch.object(G, "REG_CAP_S", 1e-6):
            with self.assertRaises(OSError) as cm:
                G.run_reg(["export", G.REGISTRY_KEY, path, "/y"])
            cause = cm.exception.__cause__
            self.assertIsInstance(cause, subprocess.TimeoutExpired)
            self.assertEqual(cause.timeout, G.REG_CAP_S)
        self.assertIn(subprocess.list2cmdline(cause.cmd), str(cm.exception))

    def test_a_reg_exe_timeout_fails_the_backup_and_the_restore_by_name(self):
        backup, key, _line = _scratch_backup(self, self.tmp)
        saved = os.path.join(backup, G.REGISTRY_BACKUP)
        with mock.patch.object(G, "REG_CAP_S", 1e-6):
            with self.assertRaises(OSError) as cm:
                G.backup(self.root, stamp=lambda: "t", run_reg=G.run_reg)
            self.assertIsInstance(cm.exception.__cause__, subprocess.TimeoutExpired)
            with self.assertRaises(G.RestoreError) as cm:
                G.restore_all(backup, run_reg=G.run_reg, out=lambda _l: None)
        self.assertEqual(cm.exception.name, G.REGISTRY_BACKUP)
        self.assertIsInstance(cm.exception.error.__cause__, subprocess.TimeoutExpired)
        # The key was not touched: its export still equals the saved one.
        check = os.path.join(self.tmp, "after-timeout.reg")
        G.run_reg(["export", key, check, "/y"])
        self.assertEqual(_read(check), _read(saved))


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

    def test_the_run_line_is_one_the_game_saved_and_the_run_changes_only_it(self):
        # A settings.ini the game wrote, from a real run's backup: the run's
        # key sits in its game section with a value other than the run's,
        # and setting the run value changes that one line and no other.
        shutil.copyfile(os.path.join(REAL_BACKUP, G.SETTINGS), self.path)
        before = _read(self.path)
        self.assertNotEqual(G.get_game_setting(self.path, G.PAUSE_ON_FOCUS_LOST),
                            G.PAUSE_ON_FOCUS_LOST_RUN_VALUE)
        G.set_game_setting(self.path, G.PAUSE_ON_FOCUS_LOST, G.PAUSE_ON_FOCUS_LOST_RUN_VALUE)
        self.assertEqual(G.get_game_setting(self.path, G.PAUSE_ON_FOCUS_LOST),
                         G.PAUSE_ON_FOCUS_LOST_RUN_VALUE)
        old, new = before.splitlines(keepends=True), _read(self.path).splitlines(keepends=True)
        self.assertEqual(len(old), len(new))
        self.assertEqual(len([i for i in range(len(old)) if old[i] != new[i]]), 1)

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


class GetVideoSettingTest(unittest.TestCase):
    """get_video_setting on copies of the settings.ini a real run backed up.
    That copy's [video] section reads `framerate = 330` and `vsync = 0`, and
    no other section of it names either key."""

    REAL_VSYNC_LINE = b"vsync = 0"

    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.path = os.path.join(self._tmp.name, G.SETTINGS)
        self.real = _read(os.path.join(REAL_BACKUP, G.SETTINGS))
        self.assertEqual(self.real.count(self.REAL_VSYNC_LINE), 1)

    def tearDown(self):
        self._tmp.cleanup()

    def test_reads_vsync_and_framerate_from_a_real_copy(self):
        _write(self.path, self.real)
        self.assertEqual(G.get_video_setting(self.path, G.VSYNC), "0")
        self.assertEqual(G.get_video_setting(self.path, G.FRAMERATE), "330")
        # The value comes from the file: the same copy with its vsync line
        # changed reads the new value.
        _write(self.path, self.real.replace(self.REAL_VSYNC_LINE, b"vsync = 1"))
        self.assertEqual(G.get_video_setting(self.path, G.VSYNC), "1")

    def test_a_missing_key_reads_none(self):
        _write(self.path, self.real.replace(self.REAL_VSYNC_LINE + b"\r\n", b"")
               .replace(self.REAL_VSYNC_LINE + b"\n", b""))
        self.assertNotIn(self.REAL_VSYNC_LINE, _read(self.path))
        self.assertIsNone(G.get_video_setting(self.path, G.VSYNC))

    def test_a_same_named_key_in_another_section_is_not_read(self):
        # The copy's vsync line taken out of [video] and put in [game]: the
        # reader finds none in [video].
        moved = self.real.replace(self.REAL_VSYNC_LINE, b"").replace(
            b"[game]", b"[game]\r\nvsync = 1", 1)
        _write(self.path, moved)
        self.assertIsNone(G.get_video_setting(self.path, G.VSYNC))
        # And with [video]'s own line kept, the [game] one does not win.
        _write(self.path, self.real.replace(b"[game]", b"[game]\r\nvsync = 1", 1))
        self.assertEqual(G.get_video_setting(self.path, G.VSYNC), "0")

    def test_two_lines_in_the_section_are_refused(self):
        _write(self.path, self.real.replace(self.REAL_VSYNC_LINE,
                                            self.REAL_VSYNC_LINE + b"\r\nvsync = 1"))
        with self.assertRaises(G.SettingNotFound):
            G.get_video_setting(self.path, G.VSYNC)


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


if __name__ == "__main__":
    unittest.main()
