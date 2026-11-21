"""Tests for mono_facts.py.

The class-shape tests run the real reader, mono_reader.ps1, on .NET assemblies
that ship with PowerShell 7, and turn what it prints into facts with the real
code. The build_facts tests that need the game's three named classes feed it a
hand-written copy of what the reader prints for them, because no game assembly
is read by a unit test.
"""

from __future__ import annotations

import copy
import functools
import os
import re
import shutil
import subprocess

import pytest

from tools.ch_mcp import dump_facts
from tools.ch_mcp import mono_facts
from tools.ch_mcp.dump_facts import FactsError

_REPO_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", ".."))

SCRAMBLED = "ʷʲʺʷʻʳʾʶˁˀʷ"
SCRAMBLED_CLASS = "ʺˀʿʷʼʻʺʴʻʲˀ"

# A real game folder's layout, copied from the 6142 install (its
# Clone Hero_Data\version.json only).
REAL_GAME_DIR = os.path.join(_REPO_ROOT, "testdata", "ch_runs", "ch-dumps-6142", "game")

# Whole classes copied unchanged from hydra-data\ch-dumps\6142\dump.cs, one for
# each way that dump writes a parameter marker.
REAL_DUMP_MARKERS = os.path.join(_REPO_ROOT, "testdata", "ch_runs", "ch-dumps-6142", "6142", "dump-param-markers.cs")


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


def _runtime_dll(dll_name):
    pwsh = shutil.which("pwsh")
    assert pwsh, "pwsh (PowerShell 7) must be on PATH"
    return os.path.join(os.path.dirname(pwsh), dll_name)


@functools.lru_cache(maxsize=None)
def _runtime_output(*dll_names):
    """The real reader's output for assemblies that ship with PowerShell 7."""
    return mono_facts.run_reader([_runtime_dll(n) for n in dll_names])


def _runtime_pairs(*dll_names):
    """Each real reader type beside the facts class the real code makes of it."""
    output = _runtime_output(*dll_names)
    return [(t, mono_facts.to_class(t)) for a in output["assemblies"] for t in a["types"]]


def test_top_level_keys_and_version():
    facts = mono_facts.build_facts(READER_OUTPUT, "v1.0.0.4045-master")
    assert facts["game_version"] == "v1.0.0.4045-master"
    files = [a["file"] for a in READER_OUTPUT["assemblies"]]
    assert facts["dump_file"].split(", ") == files
    types = [t for a in READER_OUTPUT["assemblies"] for t in a["types"]]
    assert [(c["name"], c["typedef_index"]) for c in facts["classes"]] == [(t["name"], t["row"]) for t in types]


def test_fields_have_no_offset_and_constants_are_left_out():
    pairs = _runtime_pairs("System.Collections.dll")
    constants = 0
    for raw, cls in pairs:
        kept = {f["name"]: f for f in cls["fields"]}
        for f in raw["fields"]:
            if f["literal"]:
                constants += 1
                assert f["name"] not in kept, (raw["name"], f["name"])
            else:
                assert kept[f["name"]]["offset"] is None
                assert kept[f["name"]]["static"] == f["static"]
                assert kept[f["name"]]["type"] == f["type"]
        # An enum's members are constants, so only its value field is left.
        if cls["kind"] == "enum":
            assert [f["name"] for f in cls["fields"]] == ["value__"], raw["name"]
    assert constants > 0, "the assembly must hold constants for the test to drop"
    assert any(cls["kind"] == "enum" for _, cls in pairs)


def test_methods_have_no_rva_and_keep_param_types():
    pairs = _runtime_pairs("System.Collections.dll")
    by_ref = 0
    for raw, cls in pairs:
        assert [(m["name"], m["return"], m["params"], m["static"]) for m in cls["methods"]] == [
            (m["name"], m["return"], m["params"], m["static"]) for m in raw["methods"]
        ]
        assert all(m["rva"] is None for m in cls["methods"])
        by_ref += sum(1 for m in cls["methods"] for p in m["params"] if p.split(" ", 1)[0] in ("out", "in", "ref"))
    assert by_ref > 0, "the assembly must hold by-reference parameters"


def test_row_number_is_the_typedef_index():
    # Rows restart in each assembly, so two assemblies give two classes per
    # low row, and one assembly never repeats one.
    output = _runtime_output("System.Collections.dll", "System.Private.CoreLib.dll")
    per_assembly = [[mono_facts.to_class(t)["typedef_index"] for t in a["types"]] for a in output["assemblies"]]
    for indexes in per_assembly:
        assert indexes == list(range(1, len(indexes) + 1))
    facts_classes = [mono_facts.to_class(t) for a in output["assemblies"] for t in a["types"]]
    assert sum(1 for c in facts_classes if c["typedef_index"] == 1) == len(output["assemblies"])


def test_kinds_and_bases_follow_the_dump_cs_shape():
    pairs = _runtime_pairs("System.Collections.dll", "System.Private.CoreLib.dll")
    for raw, cls in pairs:
        assert cls["kind"] == mono_facts.class_kind(raw)
        assert cls["interfaces"] == raw["interfaces"]
        # dump.cs prints a base only for a class whose base is not System.Object.
        if cls["kind"] == "class" and raw["base_full"] != "System.Object":
            assert cls["base"] == raw["base"], raw["name"]
        else:
            assert cls["base"] is None, raw["name"]
    kinds = {cls["kind"] for _, cls in pairs}
    assert kinds == {"class", "struct", "enum", "interface"}
    assert any(cls["base"] is not None for _, cls in pairs)


def test_readable_and_scrambled_names():
    pairs = _runtime_pairs("System.Collections.dll")
    for raw, cls in pairs:
        assert cls["readable"] is dump_facts.is_readable(raw["name"])


def test_signatures_name_the_three_readable_classes_only():
    facts = mono_facts.build_facts(READER_OUTPUT, "v")
    sigs = facts["signatures"]
    assert sorted(sigs) == sorted(dump_facts.NAMED_SIGNATURES)
    types = {t["name"]: t for a in READER_OUTPUT["assemblies"] for t in a["types"]}
    for name, sig in sigs.items():
        raw = types[name]
        kept = [f for f in raw["fields"] if not f["literal"]]
        assert sig["typedef_index"] == raw["row"]
        assert sig["field_count"] == len(kept)
        assert sig["method_count"] == len(raw["methods"])
        assert [f["name"] for f in sig["readable_fields"]] == [f["name"] for f in kept if dump_facts.is_readable(f["name"])]
    assert any(not dump_facts.is_readable(f["name"]) for f in types["GameManager"]["fields"])


def test_a_missing_named_class_refuses():
    # A real assembly that is not the game's holds none of the named classes.
    output = copy.deepcopy(_runtime_output("System.Collections.dll"))
    classes = [mono_facts.to_class(t) for a in output["assemblies"] for t in a["types"]]
    first = dump_facts.NAMED_SIGNATURES[0]
    with pytest.raises(FactsError) as owner:
        dump_facts.find_by_name(classes, first)
    with pytest.raises(FactsError) as caught:
        mono_facts.build_facts(output, "v")
    assert str(caught.value) == str(owner.value)
    assert first in str(caught.value)


def test_a_missing_assembly_is_named(tmp_path):
    for missing in mono_facts.GAME_ASSEMBLIES:
        folder = tmp_path / missing.replace(".", "_")
        folder.mkdir()
        for name in mono_facts.GAME_ASSEMBLIES:
            if name != missing:
                (folder / name).write_bytes(b"")
        with pytest.raises(FactsError) as caught:
            mono_facts.assembly_paths(str(folder))
        message = str(caught.value)
        assert str(folder) in message
        assert re.findall(r"[\w.-]+\.dll", message.replace(str(folder), "")) == [missing]


def test_the_managed_folder_must_be_one(tmp_path):
    data_dirs = [d for d in os.listdir(REAL_GAME_DIR) if d.endswith("_Data")]
    assert len(data_dirs) == 1, data_dirs
    game = tmp_path / "game"
    (game / data_dirs[0]).mkdir(parents=True)
    with pytest.raises(FactsError) as none:
        mono_facts.managed_folder(str(game))
    assert str(game) in str(none.value)
    managed = game / data_dirs[0] / "Managed"
    managed.mkdir()
    assert mono_facts.managed_folder(str(game)) == str(managed)
    (game / ("Other" + data_dirs[0][data_dirs[0].index("_"):]) / "Managed").mkdir(parents=True)
    with pytest.raises(FactsError) as two:
        mono_facts.managed_folder(str(game))
    assert str(game) in str(two.value)
    assert str(two.value) != str(none.value)


def test_the_reader_cap_stops_with_a_line():
    # The real reader, run with a cap far below the time it takes to start.
    caps = []

    def short_cap_runner(cmd, **kwargs):
        caps.append(kwargs["timeout"])
        return subprocess.run(cmd, **{**kwargs, "timeout": 0.05})

    with pytest.raises(FactsError) as caught:
        mono_facts.run_reader([_runtime_dll("System.Collections.dll")], runner=short_cap_runner)
    assert caps == [mono_facts.READER_CAP_S]
    message = str(caught.value)
    assert os.path.basename(mono_facts.READER_SCRIPT) in message
    assert f"{mono_facts.READER_CAP_S:.0f} s" in message


def test_a_failed_reader_run_reports_its_error():
    # A native dll beside pwsh has no .NET metadata, so the real reader fails.
    runs = []

    def kept_runner(cmd, **kwargs):
        runs.append(subprocess.run(cmd, **kwargs))
        return runs[-1]

    with pytest.raises(FactsError) as caught:
        mono_facts.run_reader([_runtime_dll("hostfxr.dll")], runner=kept_runner)
    [proc] = runs
    assert proc.returncode != 0
    message = str(caught.value)
    assert message == mono_facts.reader_failure(proc)
    assert os.path.basename(mono_facts.READER_SCRIPT) in message
    said = [text.strip() for text in (proc.stderr, proc.stdout) if text and text.strip()]
    assert said, "the failed reader said nothing"
    assert said[0] in message


def test_the_real_reader_decodes_a_runtime_assembly():
    dll = _runtime_dll("System.Collections.dll")
    output = mono_facts.run_reader([dll])
    assert [a["file"] for a in output["assemblies"]] == [os.path.basename(dll)]
    types = output["assemblies"][0]["types"]
    # ECMA-335: TypeDef rows count from 1, and row 1 is the <Module> type.
    assert [t["row"] for t in types] == list(range(1, len(types) + 1))
    assert types[0]["name"] == "<Module>"
    names = {t["name"].split("<", 1)[0] for t in types}
    nested = [t for t in types if "." in t["name"].split("<", 1)[0]]
    assert nested, "the assembly must hold nested types"
    for t in nested:
        # A nested type is written Outer.Inner, as dump.cs writes it.
        assert t["name"].split("<", 1)[0].rsplit(".", 1)[0] in names, t["name"]
    for t in types:
        if mono_facts.class_kind(t) == "enum":
            # ECMA-335: an enum holds one instance field, value__.
            assert [f["name"] for f in t["fields"] if not f["static"]] == ["value__"], t["name"]


def _markers(param):
    """The marker words in front of a parameter's type: [In], [Out], out, in or ref."""
    words = []
    for word in param.split(" "):
        if word.startswith("[") or word in ("out", "in", "ref"):
            words.append(word)
        else:
            break
    return tuple(words)


def test_the_real_reader_writes_parameter_markers_the_way_dump_cs_does():
    with open(REAL_DUMP_MARKERS, encoding="utf-8") as f:
        dump_classes = dump_facts.parse_dump(f.read())
    dump_forms = {_markers(p) for c in dump_classes for m in c["methods"] for p in m["params"]} - {()}
    output = _runtime_output("System.Collections.dll", "System.Private.CoreLib.dll")
    reader_forms = {
        _markers(p) for a in output["assemblies"] for t in a["types"] for m in t["methods"] for p in m["params"]
    } - {()}
    assert reader_forms, "the runtime assemblies must hold marked parameters"
    assert reader_forms <= dump_forms, reader_forms - dump_forms
    # In both, a bracket marker and a by-reference word never mix.
    for form in dump_forms | reader_forms:
        assert all(w.startswith("[") for w in form) or len(form) == 1, form
