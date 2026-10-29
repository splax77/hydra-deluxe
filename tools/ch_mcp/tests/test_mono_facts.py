"""Tests for mono_facts.py.

Most tests feed build_facts a hand-written copy of what mono_reader.ps1 prints,
so they never touch the game. One test runs the real reader on a small
assembly that ships with PowerShell 7.
"""

from __future__ import annotations

import copy
import os
import shutil
import subprocess

import pytest

from tools.ch_mcp import mono_facts
from tools.ch_mcp.dump_facts import FactsError

SCRAMBLED = "ʷʲʺʷʻʳʾʶˁˀʷ"
SCRAMBLED_CLASS = "ʺˀʿʷʼʻʺʴʻʲˀ"


def _type(row, name, base_full, base=None, *, namespace="", interface=False, interfaces=(), fields=(), methods=()):
    return {
        "assembly": "Assembly-CSharp.dll",
        "row": row,
        "namespace": namespace,
        "name": name,
        "is_interface": interface,
        "base": base,
        "base_full": base_full,
        "interfaces": list(interfaces),
        "fields": list(fields),
        "methods": list(methods),
    }


def _field(name, type_, static=False, literal=False):
    return {"name": name, "type": type_, "static": static, "literal": literal}


def _method(name, ret, params=(), static=False):
    return {"name": name, "return": ret, "params": list(params), "static": static}


READER_OUTPUT = {
    "assemblies": [
        {
            "file": "Assembly-CSharp.dll",
            "types": [
                _type(1, "<Module>", None),
                _type(
                    562,
                    "GameManager",
                    "UnityEngine.MonoBehaviour",
                    "MonoBehaviour",
                    fields=[
                        _field(SCRAMBLED, "GameManager", static=True),
                        _field("songTime", "double"),
                        _field("playerObjects", "BasePlayer[]"),
                        _field("MaxPlayers", "int", static=True, literal=True),
                    ],
                    methods=[
                        _method(".ctor", "void"),
                        _method(SCRAMBLED, "bool", ["out int", "List<string>"], static=True),
                    ],
                ),
                _type(216, "ScoreManager", "UnityEngine.MonoBehaviour", "MonoBehaviour"),
                _type(628, "DrumsPlayer", None, "BasePlayer<" + SCRAMBLED_CLASS + ">", interfaces=["IDisposable"]),
                _type(3, SCRAMBLED_CLASS, "System.Object", "Object", interfaces=["IComparable"]),
                _type(
                    4,
                    "NoteType",
                    "System.Enum",
                    "Enum",
                    fields=[_field("value__", "int"), _field("Kick", "NoteType", static=True, literal=True)],
                ),
                _type(5, "ExportSongList.SongExportFormat", "System.ValueType", "ValueType", interfaces=["IEquatable<int>"]),
                _type(6, "IHitTarget", None, interface=True, interfaces=["IDisposable"]),
            ],
        },
        {"file": "StrikeCore.dll", "types": [_type(1, "<Module>", None)]},
    ]
}


def _by_name(facts, name):
    return next(c for c in facts["classes"] if c["name"] == name)


def test_top_level_keys_and_version():
    facts = mono_facts.build_facts(READER_OUTPUT, "v1.0.0.4045-master")
    assert facts["game_version"] == "v1.0.0.4045-master"
    assert facts["dump_file"] == "Assembly-CSharp.dll, StrikeCore.dll"
    assert [c["name"] for c in facts["classes"]][:3] == ["<Module>", "GameManager", "ScoreManager"]
    assert len(facts["classes"]) == 9


def test_fields_have_no_offset_and_constants_are_left_out():
    gm = _by_name(mono_facts.build_facts(READER_OUTPUT, "v"), "GameManager")
    assert gm["fields"] == [
        {"name": SCRAMBLED, "type": "GameManager", "offset": None, "static": True},
        {"name": "songTime", "type": "double", "offset": None, "static": False},
        {"name": "playerObjects", "type": "BasePlayer[]", "offset": None, "static": False},
    ]


def test_methods_have_no_rva_and_keep_param_types():
    gm = _by_name(mono_facts.build_facts(READER_OUTPUT, "v"), "GameManager")
    assert gm["methods"][1] == {
        "name": SCRAMBLED,
        "return": "bool",
        "params": ["out int", "List<string>"],
        "static": True,
        "rva": None,
    }


def test_row_number_is_the_typedef_index():
    gm = _by_name(mono_facts.build_facts(READER_OUTPUT, "v"), "GameManager")
    assert gm["typedef_index"] == 562


def test_kinds_and_bases_follow_the_dump_cs_shape():
    facts = mono_facts.build_facts(READER_OUTPUT, "v")
    shapes = {c["name"]: (c["kind"], c["base"], c["interfaces"]) for c in facts["classes"]}
    assert shapes["GameManager"] == ("class", "MonoBehaviour", [])
    assert shapes["DrumsPlayer"] == ("class", "BasePlayer<" + SCRAMBLED_CLASS + ">", ["IDisposable"])
    assert shapes[SCRAMBLED_CLASS] == ("class", None, ["IComparable"])
    assert shapes["NoteType"] == ("enum", None, [])
    assert shapes["ExportSongList.SongExportFormat"] == ("struct", None, ["IEquatable<int>"])
    assert shapes["IHitTarget"] == ("interface", None, ["IDisposable"])
    assert _by_name(facts, "NoteType")["fields"] == [
        {"name": "value__", "type": "int", "offset": None, "static": False}
    ]


def test_readable_and_scrambled_names():
    facts = mono_facts.build_facts(READER_OUTPUT, "v")
    assert _by_name(facts, "GameManager")["readable"] is True
    assert _by_name(facts, SCRAMBLED_CLASS)["readable"] is False


def test_signatures_name_the_three_readable_classes_only():
    sigs = mono_facts.build_facts(READER_OUTPUT, "v")["signatures"]
    assert sorted(sigs) == ["DrumsPlayer", "GameManager", "ScoreManager"]
    gm = sigs["GameManager"]
    assert gm["typedef_index"] == 562
    assert gm["field_count"] == 3
    assert gm["method_count"] == 2
    assert [f["name"] for f in gm["readable_fields"]] == ["songTime", "playerObjects"]


def test_a_missing_named_class_refuses():
    output = copy.deepcopy(READER_OUTPUT)
    output["assemblies"][0]["types"] = [t for t in output["assemblies"][0]["types"] if t["name"] != "DrumsPlayer"]
    with pytest.raises(FactsError, match="class DrumsPlayer matched 0 classes"):
        mono_facts.build_facts(output, "v")


def test_a_missing_assembly_is_named(tmp_path):
    for name in mono_facts.GAME_ASSEMBLIES[:-1]:
        (tmp_path / name).write_bytes(b"")
    with pytest.raises(FactsError, match="missing CHNet.Server.dll"):
        mono_facts.assembly_paths(str(tmp_path))


def test_the_managed_folder_must_be_one(tmp_path):
    with pytest.raises(FactsError, match="found 0"):
        mono_facts.managed_folder(str(tmp_path))
    (tmp_path / "Game_Data" / "Managed").mkdir(parents=True)
    assert mono_facts.managed_folder(str(tmp_path)) == str(tmp_path / "Game_Data" / "Managed")


def test_the_reader_cap_stops_with_a_line():
    def fake_runner(cmd, **kwargs):
        assert kwargs["timeout"] == mono_facts.READER_CAP_S
        raise subprocess.TimeoutExpired(cmd, kwargs["timeout"])

    with pytest.raises(FactsError, match="mono_reader.ps1 did not finish in 300 s"):
        mono_facts.run_reader(["x.dll"], runner=fake_runner)


def test_a_failed_reader_run_reports_its_error():
    def fake_runner(cmd, **kwargs):
        return subprocess.CompletedProcess(cmd, 1, stdout="", stderr="BadImageFormatException")

    with pytest.raises(FactsError, match=r"failed \(exit 1\): BadImageFormatException"):
        mono_facts.run_reader(["x.dll"], runner=fake_runner)


def test_the_real_reader_decodes_a_runtime_assembly():
    pwsh = shutil.which("pwsh")
    assert pwsh, "pwsh (PowerShell 7) must be on PATH"
    dll = os.path.join(os.path.dirname(pwsh), "System.Collections.dll")
    output = mono_facts.run_reader([dll])
    assert output["assemblies"][0]["file"] == "System.Collections.dll"
    types = {t["name"]: t for t in output["assemblies"][0]["types"]}

    sd = types["SortedDictionary<TKey, TValue>"]
    assert sd["namespace"] == "System.Collections.Generic"
    assert sd["base_full"] == "System.Object"
    assert {"name": "_keys", "type": "SortedDictionary.KeyCollection<TKey, TValue>", "static": False, "literal": False} in sd["fields"]
    assert {"name": "TryGetValue", "return": "bool", "params": ["TKey", "out TValue"], "static": False} in sd["methods"]
    assert {"name": "CopyTo", "return": "void", "params": ["KeyValuePair<TKey, TValue>[]", "int"], "static": False} in sd["methods"]

    enumerator = types["SortedDictionary.Enumerator<TKey, TValue>"]
    assert mono_facts.class_kind(enumerator) == "struct"


def _runtime_methods(dll_name, type_name):
    pwsh = shutil.which("pwsh")
    assert pwsh, "pwsh (PowerShell 7) must be on PATH"
    output = mono_facts.run_reader([os.path.join(os.path.dirname(pwsh), dll_name)])
    types = [t for t in output["assemblies"][0]["types"] if t["name"] == type_name]
    return [m for t in types for m in t["methods"]]


def test_the_real_reader_writes_parameter_markers_the_way_dump_cs_does():
    # dump.cs prefixes a plain parameter with [Out] or [In], and a by-reference
    # one with out, in or ref; the two never mix.
    get_locales = [m for m in _runtime_methods("System.Private.CoreLib.dll", "Interop.Globalization") if m["name"] == "GetLocales"]
    assert {"name": "GetLocales", "return": "int", "params": ["[Out] char[]", "int"], "static": True} in get_locales

    get_bytes = [m for m in _runtime_methods("System.Memory.dll", "EncodingExtensions") if m["name"] == "GetBytes"]
    assert ["Encoding", "in ReadOnlySequence<char>", "Span<byte>"] in [m["params"] for m in get_bytes]
